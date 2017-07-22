#include <glm/gtc/type_ptr.hpp>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "aircraft/Aircraft.h"
#include "terrain/Sky.h"
#include "render/DepthMapRenderer.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
Aircraft::Aircraft(const glm::vec3& position, const glm::quat& orientation) :
    fuselage_shader_("shaders/aircraft.vs", "shaders/aircraft.fs"),
    canopy_shader_("shaders/aircraft.vs", "shaders/canopy.fs"),
    exhaust_shader_("shaders/exhaust.vs", "shaders/exhaust.fs"),
    model_("../../../assets/models/FA-22_Raptor/FA-22_Raptor.obj"),
    position_(position), orientation_(orientation), 
    lin_momentum_(AircraftToWorld(glm::vec3(27000.0f * 150.0f, 0.0f, 0.0f), 
          orientation)), 
    ang_momentum_(0.0f, 0.0f, 0.0f),
    acceleration_(0.0f, 0.0f, 0.0f) {
  // Draw the canopy last since it's transparent
  std::vector<unsigned int> draw_order(22);
  std::iota(draw_order.begin(), draw_order.end(), 0);
  draw_order.back() = 2;
  draw_order[2] = draw_order.size() - 1;
  model_.SetDrawOrder(draw_order);
  
  // Set the shader pointers for each mesh
  std::vector<Shader*> shaders(22);
  for (size_t i = 0; i < shaders.size()-1; i++) {
    shaders[i] = &fuselage_shader_;
  }
  shaders.back() = &canopy_shader_;
  model_.SetShaders(shaders);
  
  // Set the physical dimensions of the aircraft
  mass_ = 27000.0f; 
  delta_center_of_mass_ = glm::vec3(0.0f, 0.0f, -0.25f);
  inertia_[0][0] = 22000.0f;       // I_xx
  inertia_[1][1] = 162000.0f;      // I_yy
  inertia_[2][2] = 178000.0f;      // I_zz
  inertia_[0][2] = -2874.0f;       // I_xz
  inertia_[2][0] = inertia_[0][2]; // I_zx
  wetted_area_ = 316.0f;
  chord_ = 5.75f;
  span_ = 13.56f;
  dx_cg_x_ax_ = 0.05f;
  r_tail_ = glm::vec3(-4.8f, 0.0f, 0.0f);
  max_thrust_ = 311000.0f;
  
  // Define the aerodynamic coefficients
  CL_ = {0.26, 0.1, 0.2, 0.24, 0.07, 0.0, 
    -0.07, -0.14, -0.2, -0.1, -0.2, -0.3, 
    0.0, 0.55, 0.25, 0.2, 0.14, 0.07, 0.0,
    -0.07, -0.14, -0.2, -0.1, -0.2, 0.0};
  CD_ = {0.03, 0.11, 0.2, 0.4, 0.6, 0.8, 
    1.0, 0.8, 0.6, 0.4, 0.25, 0.11, // (-pi/2, 0]
    0.03, 0.11, 0.25, 0.4, 0.6, 0.8, // (0, pi/2]
    1.0, 0.8, 0.6, 0.4, 0.25, 0.11, 0.03}; // (pi/2, pi]
  CL_Q_ = 0.0f;
  Cm_Q_ = -3.6f;
  CL_alpha_dot_ = 0.72f;
  Cm_alpha_dot_ = -1.1f;
  float e = 1.0f / (1.05f + 0.007f * M_PI * span_ / chord_);
  CDi_CL2_ = 1.0f / (M_PI * e * span_ / chord_);
  CY_beta_ = -0.98f;
  Cl_beta_ = -0.12f;
  Cl_P_ = -0.26f; 
  Cl_R_ = 0.14f; 
  Cn_beta_ = 0.25f; 
  Cn_P_ = 0.022f; 
  Cn_R_ = -0.35f; 
  CL_de_ = 0.36f; 
  CD_de_ = 0.08f; 
  CY_dr_ = 0.17f; 
  Cm_de_ = -0.5f; 
  Cl_da_ = 0.08f; 
  Cn_da_ = 0.06f; 
  Cl_dr_ = -0.001f; 
  Cn_dr_ = 0.232f; 
  
  // Set the initial values for control inputs
  rudder_position_   = 0.0f;
  elevator_position_ = 0.0f;
  aileron_position_  = 0.0f;
  throttle_position_ = 1.0f;

  // Set the angle of attack so the plane is in level flight
  float dCL_dalpha0 = (CL_[1] - CL_[0]) / (2 * M_PI / (CL_.size() - 1));
  float vt = glm::l2Norm(lin_momentum_) / mass_;
  float q = 0.5f * 1.225f * vt * vt;
  float alpha0 = (mass_ * 9.81f / q / wetted_area_ - CL_[0]) / dCL_dalpha0;
  orientation_ = glm::angleAxis(alpha0,
      AircraftToWorld(glm::vec3(0.0f, 1.0f, 0.0f), orientation_)) * 
    orientation_;

  // Design Cm_ so the aircraft is stable
  glm::vec3 omega(0.0f, 0.0f, 0.0f);
  float lift0 = CalcLift(alpha0, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag0 = CalcDrag(lift0, alpha0, vt, 0.0f, q, 0.0f);
  float M_LD0 = dx_cg_x_ax_ * chord_ * (lift0*cos(alpha0) + drag0*sin(alpha0));
  float alpha1 = alpha0 + glm::radians(0.01f);
  float lift1 = CalcLift(alpha1, 0.0f, omega, vt, 0.0f, q, 0.0f);
  float drag1 = CalcDrag(lift1, alpha1, vt, 0.0f, q, 0.0f);
  float M_LD1 = dx_cg_x_ax_ * chord_ * (lift1*cos(alpha1) + drag1*sin(alpha1));
  float dCm_LD_dalpha = (M_LD1-M_LD0)/(alpha1-alpha0)/q/wetted_area_/ chord_;
  float dCm_dalpha = 1.6f * dCm_LD_dalpha;
  float Cm0 = -M_LD0 / q / wetted_area_ / chord_ - dCm_dalpha * alpha0;
  Cm_ = {Cm0 - dCm_dalpha * (float)M_PI, Cm0, Cm0 + dCm_dalpha * (float)M_PI};

  // Set up the data for drawing the exhaust
  delta_exhaust_ = {0.637885f, -6.717596f, -0.562625f};
  delta_flame_ = {0.637885f, -6.217596f, -0.593625f};
  r_flame_ = 0.55f;
  SetupDrawData();
}

//****************************************************************************80
Aircraft::~Aircraft() {
  glDeleteVertexArrays(1, &sphere_VAO_);
}

//****************************************************************************80
void Aircraft::Draw(Camera const& camera, const Sky& sky, 
    const DepthMapRenderer& depthmap_renderer, const Shader* shader) {
  if (!shader) {
    // Send data to the shaders
    SetShaderData(camera, sky, depthmap_renderer);
  }
  else {
    shader->Use();
    // Send the model info
    glm::mat4 aircraft_model = GetAircraftModel();
    glUniformMatrix4fv(glGetUniformLocation(shader->GetProgram(), "model"), 1, 
        GL_FALSE, glm::value_ptr(aircraft_model));
  }
  // Enable face-culling (for cockpit drawing) 
  glEnable(GL_CULL_FACE);
  // Draw the model
  model_.Draw(shader);
  // Disable face-culling
  glDisable(GL_CULL_FACE);
  if (!shader) {
    // Draw the exhaust last
    DrawExhaust();
  }
}

//****************************************************************************80
void Aircraft::UpdateControls(std::vector<bool> const& keys) {
  // Elevator control
  if(keys[GLFW_KEY_UP]) {
    elevator_position_ = 0.2f * elevator_position_max_;
  }
  else if(keys[GLFW_KEY_DOWN]) {
    elevator_position_ = -0.2f * elevator_position_max_;
  }
  else {
    elevator_position_ = 0.0f;
  }
  // Aileron control
  if(keys[GLFW_KEY_RIGHT]) {
    aileron_position_ = 0.5f * aileron_position_max_;
  }
  else if(keys[GLFW_KEY_LEFT]) {
    aileron_position_ = -0.5f * aileron_position_max_;
  }
  else {
    aileron_position_ = 0.0f;
  }
  // Rudder control
  if(keys[GLFW_KEY_D]) {
    rudder_position_ = 0.5f * rudder_position_max_;
  }
  else if(keys[GLFW_KEY_A]) {
    rudder_position_ = -0.5f * rudder_position_max_;
  }
  else {
    rudder_position_ = 0.0f;
  }
  // Throttle control
  if(keys[GLFW_KEY_EQUAL]) {
    throttle_position_ += 0.005;
    throttle_position_ = std::min(1.0f, throttle_position_);
  }
  else if(keys[GLFW_KEY_MINUS]) {
    throttle_position_ -= 0.005;
    throttle_position_ = std::max(0.0f, throttle_position_);
  }
}

//****************************************************************************80
void Aircraft::operator()(const std::vector<float>& state, 
    std::vector<float>& deriv, float /* t */) {
  // Unpack the state vector
  glm::vec3 position(state[0], state[1], state[2]);
  glm::quat orientation;
  orientation.w = state[3];
  orientation.x = state[4];
  orientation.y = state[5];
  orientation.z = state[6];
  glm::vec3 lin_momentum(state[7], state[8], state[9]);
  glm::vec3 ang_momentum(state[10], state[11], state[12]);
  glm::vec3 omega = 
    glm::inverse(AircraftToWorld(inertia_, orientation)) * ang_momentum;

  // Update the forces and torques in the aircraft frame
  glm::vec3 forces, torques;
  CalcAeroForcesAndTorques(position, orientation, lin_momentum, 
      WorldToAircraft(omega, orientation), forces, torques);
  forces += CalcEngineForce();

  // Rotate forces and torques to world frame and add gravity
  forces = AircraftToWorld(forces, orientation);
  torques = AircraftToWorld(torques, orientation);
  forces += CalcGravityForce();

  // Update acceleration (for computing angle rates)
  acceleration_ = WorldToAircraft(forces / mass_, orientation);

  // Compute the derivative of the state vector
  for (int i = 0; i < 3; ++i) 
    deriv[i] = lin_momentum[i] / mass_;
  glm::quat omega_quat(0.0f, omega);
  glm::quat spin = 0.5f * omega_quat * orientation;
  deriv[3] = spin.w;
  deriv[4] = spin.x;
  deriv[5] = spin.y;
  deriv[6] = spin.z;
  for (int i = 0; i < 3; ++i) 
    deriv[i+7] = forces[i];
  for (int i = 0; i < 3; ++i) 
    deriv[i+10] = torques[i];
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void Aircraft::CalcAeroForcesAndTorques(const glm::vec3& position,
    const glm::quat& orientation, const glm::vec3& lin_momentum, 
    const glm::vec3& omega, glm::vec3& forces, 
    glm::vec3& torques) const {
  glm::vec3 va = WorldToAircraft(lin_momentum / mass_, orientation);
  float vt = glm::l2Norm(va);
  if (vt > std::numeric_limits<float>::epsilon()) {
    glm::vec3 aa = WorldToAircraft(acceleration_, orientation);
    float alpha = CalcAlpha(va);
    float beta = CalcBeta(va);
    float alpha_dot = CalcAlphaDot(va, aa); 
    float dve = CalcTailVelocity(omega); 
    float rho = 1.225f * exp(-position.y / 7300.0f);
    float q = 0.5f * rho * vt * vt;

    float lift = CalcLift(alpha, alpha_dot, omega, vt, dve, q, 
        elevator_position_);
    float drag = CalcDrag(lift, alpha, vt, dve, q, elevator_position_);
    float side = CalcSideForce(beta, q, elevator_position_);
    forces.x = lift * sin(alpha) - drag * cos(alpha) - side * sin(beta);
    forces.y = side * cos(beta);
    forces.z = -lift * cos(alpha) - drag * sin(alpha);

    torques.x = CalcRollMoment(beta, omega, vt, q, aileron_position_, 
        rudder_position_);
    torques.y = CalcPitchMoment(alpha, alpha_dot, omega, vt, dve, q, 
        elevator_position_, lift, drag);
    torques.z = CalcYawMoment(beta, omega, vt, q, aileron_position_, 
        rudder_position_);
  }
  else {
    forces = glm::vec3(0.0f, 0.0f, 0.0f);
    torques = glm::vec3(0.0f, 0.0f, 0.0f);
  }
}

//****************************************************************************80
void Aircraft::SetShaderData(const Camera& camera, const Sky& sky,
    const DepthMapRenderer& depthmap_renderer) {
  // Set material uniforms
  fuselage_shader_.Use();
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "material.specular"), 0.7f, 0.7f, 0.7f);
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "material.shiny"), 1.0f);
  
  canopy_shader_.Use();
  glUniform3f(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "material.specular"), 1.0f, 1.0f, 1.0f);
  glUniform1f(glGetUniformLocation(canopy_shader_.GetProgram(), 
        "material.shiny"), 64.0f);
  
  // Set common data
  std::vector<const Shader*> all_shaders = {&fuselage_shader_, &canopy_shader_,
    &exhaust_shader_};
  const glm::vec3& fog_color = sky.GetFogColor();
  const std::array<float,2>& fog_start_end = sky.GetFogStartEnd();
  for (const Shader* s : all_shaders) {
    s->Use();
    // Set view/projection uniforms
    glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "view"), 1, 
        GL_FALSE, glm::value_ptr(camera.GetViewMatrix()));
    glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "projection"), 1, 
        GL_FALSE, glm::value_ptr(camera.GetProjectionMatrix()));

    // Set fog uniforms
    glUniform3f(glGetUniformLocation(s->GetProgram(), "fog.Color"),
        fog_color.x, fog_color.y, fog_color.z);
    glUniform1f(glGetUniformLocation(s->GetProgram(), "fog.Start"), 
        fog_start_end[0]);
    glUniform1f(glGetUniformLocation(s->GetProgram(), "fog.End"), 
        fog_start_end[1]);
    glUniform1i(glGetUniformLocation(s->GetProgram(), "fog.Equation"), 
        sky.GetFogEquation());
  }

  // Set data for aircraft
  glm::mat4 aircraft_model = GetAircraftModel();
  std::vector<const Shader*> model_shaders = {&fuselage_shader_, 
    &canopy_shader_};
  const glm::vec3& sun_dir = sky.GetSunDirection();
  const glm::vec3& sun_color = sky.GetSunColor();
  glm::vec3 camera_pos = camera.GetPosition();
  for (const Shader* s : model_shaders) {
    s->Use();
    // Set model uniforms
    glUniformMatrix4fv(glGetUniformLocation(s->GetProgram(), "model"), 1, 
        GL_FALSE, glm::value_ptr(aircraft_model));

    // Set lighting uniforms
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.direction"),
        sun_dir.x, sun_dir.y, sun_dir.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.ambient"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.diffuse"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);
    glUniform3f(glGetUniformLocation(s->GetProgram(), "light.specular"), 
        0.7*sun_color.x, 0.7*sun_color.y, 0.7*sun_color.z);

    // Set the camera position uniform
    glUniform3f(glGetUniformLocation(s->GetProgram(), "viewPos"), 
        camera_pos.x, camera_pos.y, camera_pos.z);
  }

  // Set data for the engine flame
  glm::vec3 flame_color(1.0f, 0.76f, 0.44f);
  flame_color += 0.1*(float)rand()/(float)(RAND_MAX);
  fuselage_shader_.Use();
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame_color"), flame_color.x, flame_color.y, flame_color.z);
  
  glm::mat4 flame_model = glm::translate(glm::mat4(), position_);
  flame_model = glm::translate(flame_model, delta_center_of_mass_);
  flame_model *= glm::toMat4(orientation_);
  flame_model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
        glm::vec3(0.0f, 0.0f, 1.0f)));
  flame_model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)));
  flame_model = glm::translate(flame_model, -delta_center_of_mass_);
  glm::vec3 flame1_pos = delta_flame_;
  glm::vec3 flame2_pos = delta_flame_;
  flame2_pos.x = -flame2_pos.x;
  glm::vec4 tmp = flame_model * glm::vec4(flame1_pos, 1.0f);
  tmp += 0.01*(float)rand()/(float)(RAND_MAX);
  for (int d = 0; d < 3; ++d) flame1_pos[d] = tmp[d];
  tmp = flame_model * glm::vec4(flame2_pos, 1.0f);
  tmp += 0.01*(float)rand()/(float)(RAND_MAX);
  for (int d = 0; d < 3; ++d) flame2_pos[d] = tmp[d];
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame1_pos"), flame1_pos.x, flame1_pos.y, flame1_pos.z);
  glUniform3f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame2_pos"), flame2_pos.x, flame2_pos.y, flame2_pos.z);
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "r_flame"), r_flame_);
  GLfloat flame_alpha = std::pow(throttle_position_, 5.0); 
  glUniform1f(glGetUniformLocation(fuselage_shader_.GetProgram(), 
        "flame_alpha"), flame_alpha);      
}

//****************************************************************************80
void Aircraft::SetupDrawData() {
  std::vector<GLfloat> vertices;
  std::vector<GLuint> indices;
  GLuint nstacks = 10;
  GLuint nslices = 10;
  float r = 1.0f;
  float sStep = M_PI / (float) nstacks;
  std::vector<float> t(nslices+1);
  t[0] = -M_PI/2;
  t[1] = t[0] + M_PI / 4;
  t.back() = M_PI/2;
  float dt = 2.0 * (M_PI/2 - (t[1] - t[0])) / (nslices - 2);
  for (GLuint i = 2; i < nslices; ++i) {
    t[i] = t[i-1] + dt;
  }
  for (GLuint i = 0; i < nslices; ++i) {
    for (float s = -M_PI; s <= M_PI-0.0001f; s += sStep) {
      float rt = r * (4.0f * t[i]*t[i]/M_PI/M_PI + 1.0f);
      vertices.push_back(rt * cos(t[i]) * cos(s));
      vertices.push_back(r * sin(t[i]));
      vertices.push_back(rt * cos(t[i]) * sin(s));

      rt = r * (4.0f * t[i+1]*t[i+1]/M_PI/M_PI + 1.0f);
      vertices.push_back(rt * cos(t[i+1]) * cos(s));
      vertices.push_back(r * sin(t[i+1]));
      vertices.push_back(rt * cos(t[i+1]) * sin(s));
    }
  }
  for (std::size_t i = 0; i < vertices.size()/3; i++) {
    indices.push_back((GLuint)i);
  }
  sphere_numindices_ = static_cast<GLuint>(indices.size());

  GLuint VBO, EBO;
  glGenVertexArrays(1, &sphere_VAO_);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  
  glBindVertexArray(sphere_VAO_);

  // Set up the VBO
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * vertices.size(), 
      vertices.data(), GL_STATIC_DRAW);
  
  // Set up the EBO
  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLuint) * indices.size(), 
      indices.data(), GL_STATIC_DRAW);

  GLint pos_loc  = glGetAttribLocation(exhaust_shader_.GetProgram(), 
      "position");
 
  // Position attribute
  glEnableVertexAttribArray(pos_loc);
  glVertexAttribPointer(pos_loc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), 
      (GLvoid*)0);

  // Unbind VBO and VAO, but not EBO
  glBindBuffer(GL_ARRAY_BUFFER, 0); 
  glBindVertexArray(0);
  glDeleteBuffers(1, &EBO); 
}

//****************************************************************************80
void Aircraft::DrawExhaust() {
  exhaust_shader_.Use();

  // Enable face-culling 
  glEnable(GL_CULL_FACE);
  glFrontFace(GL_CW);
  
  // Orient model
  glm::mat4 exhaust_model = glm::translate(glm::mat4(), position_);
  exhaust_model = glm::translate(exhaust_model, delta_center_of_mass_);
  exhaust_model *= glm::toMat4(orientation_);
  exhaust_model *= glm::toMat4(glm::angleAxis(glm::radians(90.0f), 
        glm::vec3(0.0f, 0.0f, 1.0f)));
  exhaust_model *= glm::toMat4(glm::angleAxis(glm::radians(180.0f), 
        glm::vec3(1.0f, 0.0f, 0.0f)));
  exhaust_model = glm::translate(exhaust_model, -delta_center_of_mass_);

  // Set positions, transparency, etc.
  float xs = 0.433f; // width
  float ys = 1.0f; // length
  float zs = 0.210f; // height
  zs *= 0.6 + 0.4 * throttle_position_;
  std::vector<float> lengths = {1.0f, 0.9f, 0.8f, 0.7f, 0.6f};
  std::vector<float> alphas = {0.3f, 0.2f, 0.1f, 0.05f, 0.02f};
  float tp0 = 0.6f; // throttle position where exhaust appears
  for (float& a : alphas) {
    if (throttle_position_ < tp0) 
      a *= 0.0f;
    else
      a *= (throttle_position_ - tp0) / (1.0f - tp0);
  }
  for (float& l : lengths) {
    if (throttle_position_ < tp0) 
      l *= 0.0f;
    else
      l *= std::pow(throttle_position_ - tp0, 1.0f/8.0f) /
        std::pow(1.0f - tp0, 1.0f/8.0f);
  }
  for (std::size_t i = 0; i < lengths.size(); ++i) {
    // Set exhaust color and transparency
    glm::vec3 color(0.878f, 0.597f, 0.6f);
    glm::vec3 flame_color(1.0f, 0.76f, 0.44f);
    float alpha = 0.5 + 0.5 * (float)i / (float)lengths.size();
    color = alpha * color + (1.0f - alpha) * flame_color;
    color.x += 0.1*(float)rand()/(float)(RAND_MAX);
    glUniform4f(glGetUniformLocation(exhaust_shader_.GetProgram(), 
          "exhaust_color"), color.x, color.y, color.z, alphas[i]);
 
    for (int e = 0; e < 2; ++e) {
      glm::vec3 delta_exhaust = delta_exhaust_;
      delta_exhaust.x *= -2.0f * (float)e + 1.0f;
      if (i > 0) {
        for (std::size_t ip = 0; ip < i; ++ip) {
          delta_exhaust.y -= 1.1*ys*lengths[ip];
        }
      }
      delta_exhaust += (float)i*0.01*(float)rand()/(float)(RAND_MAX);
      glm::mat4 model = glm::translate(exhaust_model, delta_exhaust);
      float xsl = xs*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      float ysl = ys*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      float zsl = zs*lengths[i] + float(i)*0.01*(float)rand()/(float)(RAND_MAX);
      model = glm::scale(model, glm::vec3(xsl, ysl, zsl));
      
      // Set model uniform for exhaust
      glUniformMatrix4fv(glGetUniformLocation(exhaust_shader_.GetProgram(), 
            "model"), 1, GL_FALSE, glm::value_ptr(model));

      // Draw the ellipsoid
      glBindVertexArray(sphere_VAO_);
      glDrawElements(GL_TRIANGLE_STRIP, sphere_numindices_, GL_UNSIGNED_INT, 0);
    }
  }
  glBindVertexArray(0);

  // Disable face-culling
  glDisable(GL_CULL_FACE);
  glFrontFace(GL_CCW);
}

} // End namespace TopFun
