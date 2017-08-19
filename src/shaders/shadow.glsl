const int MAX_NUM_CASCADES = 10;

uniform mat4 lightSpaceMatrix[MAX_NUM_CASCADES];
uniform sampler2D depthMap[MAX_NUM_CASCADES];
uniform float subfrusta_extents[MAX_NUM_CASCADES];
uniform float shadow_bias[MAX_NUM_CASCADES];
uniform int num_cascades;

uniform vec3 frustumOrigin;
uniform vec3 frustumTerminus;
uniform vec3 cameraFront;

float ShadowCalculation(vec3 fragPos, vec4 fragPosLightSpace, 
    sampler2D depthMap, float bias, vec3 lightDir, vec3 normal) {
  // perform perspective divide
  vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
  // transform to [0,1] range
  projCoords = projCoords * 0.5 + 0.5;
  // get closest depth value from light's perspective 
  // (using [0,1] range fragPosLight as coords)
  float closestDepth = texture(depthMap, projCoords.xy).r; 
  // get depth of current fragment from light's perspective
  float currentDepth = projCoords.z;
  // calculate bias (based on depth map resolution and slope)
  normal = normalize(normal);
  bias = max(bias * (1.0 - dot(normal, lightDir)), bias);
  // bias = 0.0; // allows depth map frustrum to be visualized
  // check whether current frag pos is in shadow
  // float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;
  // PCF
  float shadow = 0.0;
  vec2 texelSize = 1.0 / textureSize(depthMap, 0);
  for(int x = -1; x <= 1; ++x) {
    for(int y = -1; y <= 1; ++y) {
      float pcfDepth = 
        texture(depthMap, projCoords.xy + vec2(x, y) * texelSize).r; 
      shadow += currentDepth - bias > pcfDepth  ? 1.0 : 0.0;        
    }    
  }
  shadow /= 9.0;
  
  // keep the shadow at 0.0 when outside the far_plane
  if(projCoords.z > 1.0)
    shadow = 0.0;
      
  return shadow;
}

int GetCascadeIndex(vec3 fragPos) {
  float d = dot(cameraFront, fragPos - frustumOrigin) / 
    length(frustumTerminus - frustumOrigin);
  for (int i = 0; i < num_cascades; ++i) {
    if (d < subfrusta_extents[i]) {
      return i;
    }
  }
  return num_cascades - 1;
}
