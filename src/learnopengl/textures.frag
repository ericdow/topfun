#version 330 core
in vec3 ourColor;
in vec2 TexCoord;

out vec4 color;

// Texture samplers
uniform sampler2D ourTexture1;
uniform sampler2D ourTexture2;

void main()
{
	// Linearly interpolate between both textures (second texture is only slightly combined)
	color = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, TexCoord), 0.2) 
  * vec4(ourColor, 1.0f);
	
  // we can flip one texture by negating it's s-coordinate:
  // color = mix(texture(ourTexture1, TexCoord), texture(ourTexture2, 
  // vec2(-TexCoord.s, TexCoord.t)), 0.2) 
  // * vec4(ourColor, 1.0f);
}
