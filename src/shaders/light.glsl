struct Light {
  vec3 direction;
  vec3 ambient;
  vec3 diffuse;
  vec3 specular;
};

vec3 CalcAmbient(vec3 ambient, vec3 color) {
  return ambient * color;
}

vec3 CalcDiffuse(vec3 normal, vec3 light_dir, vec3 diffuse, vec3 color) {
  float diff = max(dot(normal, light_dir), 0.0);
  return diffuse * diff * color;
}

vec3 CalcSpecular(vec3 normal, vec3 light_dir, vec3 view_dir, vec3 specular, 
    vec3 color, float shininess) {
  vec3 reflect_dir = reflect(-light_dir, normal); 
  float spec = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
  return specular * spec * color;
} 
