#include "module/perlin.h"

#include "sky/CloudRenderer.h"
#include "render/SceneRenderer.h"
#include "utils/GLEnvironment.h"

namespace TopFun {
//****************************************************************************80
// PUBLIC FUNCTIONS
//****************************************************************************80
CloudRenderer::CloudRenderer(GLuint map_width, GLuint map_height) :
  map_width_(map_width), map_height_(map_height), 
  depth_map_shader_("shaders/depthmap.vs", "shaders/depthmap.fs"),
  raymarch_shader_("shaders/clouds.vs", "shaders/clouds_raymarch.fs"),
  blend_shader_("shaders/clouds.vs", "shaders/clouds_blend.fs"),
  depth_map_renderer_(map_width, map_height),
  cloud_start_end_({10.0f, 100.0f}), l_stop_max_(1000.0f), 
  max_cloud_height_((cloud_start_end_[1] - cloud_start_end_[0])),
  detail_({32,32,32}, "detail", {{1,1,1},{2,2,2},{3,3,3}}),
  detail_scale_(1.0f / 20.0f),
  shape_({64, 32, 64}, "shape", 
      {{{5, 4.0, 0.3}},{5,5,5},{4,4,4},{3,3,3}}), shape_scale_(1.0f / 100.0f),
  weather_scale_(1.0f / 500.0f) {

  // Check that the start and end heights of the clouds are valid
  if (cloud_start_end_[0] > cloud_start_end_[1]) {
    std::string message = "Invalid cloud star/end points\n";
    throw std::invalid_argument(message);
  }

  // Generate the weather texture
  GenerateWeatherTexture(1024);

  // Set up the quad for rendering the cloud texture
  float quadVertices[] = {
    // positions        // texture Coords
    -1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
     1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
     1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
  };
  glGenVertexArrays(1, &quadVAO_);
  glGenBuffers(1, &quadVBO_);
  glBindVertexArray(quadVAO_);
  glBindBuffer(GL_ARRAY_BUFFER, quadVBO_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, 
      GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 
      (void*)(3 * sizeof(float)));
  
  // Create cloud textures
  std::vector<GLuint*> textures = {&texture_curr_, &texture_prev_};
  for (GLuint* t : textures) {
    glGenTextures(1, t);
    glBindTexture(GL_TEXTURE_2D, *t);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, map_width_, map_height_, 0, GL_RGBA,
        GL_UNSIGNED_BYTE, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
  }
  textures = {&depth_curr_, &depth_prev_};
  for (GLuint* t : textures) {
	  glGenTextures(1, t);
    glBindTexture(GL_TEXTURE_2D, *t);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, map_width_, map_height_, 
        0, GL_DEPTH_COMPONENT, GL_UNSIGNED_BYTE, NULL);
  }

  // Attach current cloud texture as FBO's color buffer
  glGenFramebuffers(1, &cloudFBO_);
  glBindFramebuffer(GL_FRAMEBUFFER, cloudFBO_);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, 
      texture_curr_, 0);
  // Attach current cloud depth as FBO's depth buffer
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, 
      depth_curr_, 0);
  // Clean up
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
}

//****************************************************************************80
CloudRenderer::~CloudRenderer() {
  glDeleteVertexArrays(1, &quadVAO_);
}

//****************************************************************************80
void CloudRenderer::RenderToTexture(Terrain& terrain, const Sky& sky, 
    Aircraft& aircraft, const Camera& camera) {
  // Render the depth map
  glm::mat4 projection_view = camera.GetProjectionMatrix() * 
    camera.GetViewMatrix();
  depth_map_renderer_.Render(terrain, sky, aircraft, camera, projection_view,
      depth_map_shader_);
  // Set up the viewport
  glm::ivec4 viewport_orig = GLEnvironment::GetViewport();
  glViewport(0, 0, map_width_, map_height_);
  // Perform ray-marching and render the clouds to a texture
  glBindFramebuffer(GL_FRAMEBUFFER, cloudFBO_);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  SetShaderData(sky, camera);
  glBindVertexArray(quadVAO_);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // Clean up and restore viewport
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_3D, 0);
  glViewport(0, 0, viewport_orig[2], viewport_orig[3]);
  // Copy current cloud texture for next render
  glCopyImageSubData(texture_curr_, GL_TEXTURE_2D, 0, 0, 0, 0, texture_prev_,
      GL_TEXTURE_2D, 0, 0, 0, 0, map_width_, map_height_, 1);
  // Copy current cloud depth for next render
  glCopyImageSubData(depth_curr_, GL_TEXTURE_2D, 0, 0, 0, 0, depth_prev_,
      GL_TEXTURE_2D, 0, 0, 0, 0, map_width_, map_height_, 1);
}
  
//****************************************************************************80
void CloudRenderer::BlendWithScene() const {
  // Send current cloud texture to shader
  blend_shader_.Use();
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, texture_curr_);
  glUniform1i(glGetUniformLocation(blend_shader_.GetProgram(), "clouds"), 0);
  // Blend the cloud texture with the scene
  glBindVertexArray(quadVAO_);
  glEnable(GL_BLEND);
  glBlendFunc(GL_ONE, GL_SRC_ALPHA);
  glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
  // Clean up
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindVertexArray(0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindTexture(GL_TEXTURE_3D, 0);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glDisable(GL_BLEND);
}

//****************************************************************************80
// PRIVATE FUNCTIONS
//****************************************************************************80
void CloudRenderer::SetShaderData(const Sky& sky, Camera const& camera) {
  raymarch_shader_.Use();
  // Camera related data
  glm::mat4 proj_view = camera.GetProjectionMatrix() * camera.GetViewMatrix();
  glUniformMatrix4fv(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "projview"), 1, GL_FALSE, glm::value_ptr(proj_view));
  glUniformMatrix4fv(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "inv_projview"), 1, GL_FALSE, glm::value_ptr(glm::inverse(proj_view)));
  glUniformMatrix4fv(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "projview_prev"), 1, GL_FALSE, glm::value_ptr(proj_view_prev_));
  proj_view_prev_ = proj_view; 
  glm::ivec4 vp = GLEnvironment::GetViewport();
  glUniform4i(glGetUniformLocation(raymarch_shader_.GetProgram(), "viewport"), 
      vp[0], vp[1], vp[2], vp[3]);
  std::array<float,2> near_far = camera.GetNearFar();
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "camera_near"), near_far[0]);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), "camera_far"), 
      near_far[1]);
  // Depth map of the full scene
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, depth_map_renderer_.GetDepthMap());
  glUniform1i(glGetUniformLocation(raymarch_shader_.GetProgram(), "depth_map"),
      0);
  // Cloud parameters
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "cloud_start"), cloud_start_end_[0]);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), "cloud_end"), 
      cloud_start_end_[1]);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), "l_stop_max"), 
      l_stop_max_);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "max_cloud_height"), max_cloud_height_);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), "seed"), 
      (float) rand() / RAND_MAX);
  // Cloud detail texture
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_3D, detail_.GetTexture());
  glUniform1i(glGetUniformLocation(raymarch_shader_.GetProgram(), "detail"), 1);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "detail_scale"), detail_scale_);
  // Cloud shape texture
  glActiveTexture(GL_TEXTURE2);
  glBindTexture(GL_TEXTURE_3D, shape_.GetTexture());
  glUniform1i(glGetUniformLocation(raymarch_shader_.GetProgram(), "shape"), 2);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "shape_scale"), shape_scale_);
  // Cloud weather texture
  glActiveTexture(GL_TEXTURE3);
  glBindTexture(GL_TEXTURE_2D, weather_);
  glUniform1i(glGetUniformLocation(raymarch_shader_.GetProgram(), "weather"), 
      3);
  glUniform1f(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "weather_scale"), weather_scale_);
  // Cloud texture from previous render
  glActiveTexture(GL_TEXTURE4);
  glBindTexture(GL_TEXTURE_2D, texture_prev_);
  glUniform1i(glGetUniformLocation(raymarch_shader_.GetProgram(), 
        "texture_prev"), 4);
  // Sun parameters
  glm::vec3 sun_dir = sky.GetSunDirection();
  glUniform3f(glGetUniformLocation(raymarch_shader_.GetProgram(), "sun_dir"), 
      sun_dir[0], sun_dir[1], sun_dir[2]);
  glm::vec3 sun_color = sky.GetSunColor();
  glUniform3f(glGetUniformLocation(raymarch_shader_.GetProgram(), "sun_color"), 
      sun_color[0], sun_color[1], sun_color[2]);
}

//****************************************************************************80
void CloudRenderer::GenerateWeatherTexture(unsigned size) {
  // Generate the coverge and height data
  noise::module::Perlin perlin_generator;
  perlin_generator.SetOctaveCount(8);
  perlin_generator.SetFrequency(6.0);
  perlin_generator.SetPersistence(0.2);
  std::vector<unsigned char> pixels(size * size * 3);
  for (std::size_t i = 0; i < size; ++i) {
    for (std::size_t j = 0; j < size; ++j) {
      float x = (float)i / size;
      float y = (float)j / size;
      float a = perlin_generator.GetValue(x    ,y ,   0.0);
      float b = perlin_generator.GetValue(x+1.0,y ,   0.0);
      float c = perlin_generator.GetValue(x    ,y+1.0,0.0);
      float d = perlin_generator.GetValue(x+1.0,y+1.0,0.0);
      float xmix = 1.0 - x;
      float ymix = 1.0 - y;
      float x1 = glm::mix(a, b, xmix);
      float x2 = glm::mix(c, d, xmix);
      float val = glm::mix(x1, x2, ymix);
      // Clamp strictly between 0 and 1
      val = val> 1.0 ? 1.0 :val;
      val = val< 0.0 ? 0.0 :val;
      std::size_t n = size * j + i;
      pixels[3*n    ] = (unsigned char)std::round(val * 255);
      pixels[3*n + 1] = (unsigned char)std::round(val * 255);
    }
  }
  // Generate the altitude data
  // TODO
  for (std::size_t n = 0; n < size * size; ++n)
    pixels[3*n + 2] = (unsigned char)0;
  
  // Load the texture
  glGenTextures(1, &weather_);
  glBindTexture(GL_TEXTURE_2D, weather_); 
  // Set our texture parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // Bind the data
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, size, size, 0, GL_RGB, 
      GL_UNSIGNED_BYTE, (GLvoid*)pixels.data());
  glBindTexture(GL_TEXTURE_2D, 0);
}

} // End namespace TopFun

