#version 420

//  In from the vertex shader
in vec4 fColour;	
in vec4 fNormal;
in vec4 fVertWorldLocation;
in vec4 fUVx2;			// NEW
	
uniform vec4 diffuseColour;
uniform vec4 specularColour;

uniform vec4 eyeLocation;
	
//uniform float newRed;
//uniform float newGreen;
//uniform float newBlue;
// uniform vec3 newColour;	

// Used for drawing debug object that aren't lit
uniform vec4 debugColour;
uniform bool bUseDebugColourNoLight;		// float 0.0f 1.0f

// Texture samplers, etc.
uniform sampler2D texSamp2D_00;		// Raptors
uniform sampler2D texSamp2D_01;		// Brick
uniform sampler2D texSamp2D_02;		// Yellow
uniform sampler2D texSamp2D_03;
uniform vec4 textureBlendRatio;	// x = 0, y = 1, z = 2, w = 3
uniform bool bUseTexture;

uniform sampler2D texAphaBlendSampler;
uniform bool bUseTextureAlpha;

// Used for "discard" transparency
uniform bool bIsImposterUseDiscard;


out vec4 pixelColour;			// RGB + A 
                                // XYZ + W

// Fragment shader
struct sLight
{
	vec4 position;			
	vec4 diffuse;	
	vec4 specular;	// rgb = highlight colour, w = power
	vec4 atten;		// x = constant, y = linear, z = quadratic, w = DistanceCutOff
	vec4 direction;	// Spot, directional lights
	vec4 param1;	// x = lightType, y = inner angle, z = outer angle, w = TBD
	                // 0 = pointlight
					// 1 = spot light
					// 2 = directional light
	vec4 param2;	// x = 0 for off, 1 for on
};

const int POINT_LIGHT_TYPE = 0;
const int SPOT_LIGHT_TYPE = 1;
const int DIRECTIONAL_LIGHT_TYPE = 2;

const int NUMBEROFLIGHTS = 10;
uniform sLight theLights[NUMBEROFLIGHTS];  	// 80 uniforms


vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular );
	 
void main()  
{
//	gl_FragColor = vec4(newColour, 1.0f);
	
//	pixelColour = vec4(newColour, 1.0f);

	// Is an imposter:
	// Don't light
	// Use discard
	if (bIsImposterUseDiscard)
	{
		// Assume we are using only texture 0
		pixelColour.rgb 
			+= (texture( texSamp2D_00, fUVx2.xy ).rgb * textureBlendRatio.x);
		
		// Check to see how "black" the background is
		// Convert to black and white (by averaging)
		// (real greyscale is this, FYI ( (0.3 * R) + (0.59 * G) + (0.11 * B) ).)
		
		// This will be 0.0 for black and 1.0 for white 
		float bw = (pixelColour.r + pixelColour.g + pixelColour.b )/3.0f;
			
		// Set transparency to the greyscale colour
		pixelColour.a = bw;
		
//		pixelColour.a = 1.0f;
		if ( bw <= 0.10f )
		{ 
			//pixelColour.a = 0.0f;
			
			// Don't draw pixel at all
			discard;
		}
	
		return;
	}

	
	if ( bUseDebugColourNoLight )
	{
		pixelColour = debugColour;
		return;
	}
	
	vec3 materialColour = vec3(0.0f, 0.0f, 0.0f);	// Black
	
	// use textures
	if ( bUseTexture )
	{	
		materialColour.rgb 
			+= (texture( texSamp2D_00, fUVx2.xy ).rgb * textureBlendRatio.x);
		
		materialColour.rgb 
			+= (texture( texSamp2D_01, fUVx2.xy ).rgb * textureBlendRatio.y);
		
		materialColour.rgb 
			+= (texture( texSamp2D_02, fUVx2.xy ).rgb * textureBlendRatio.z);

		materialColour.rgb 
			+= (texture( texSamp2D_03, fUVx2.xy ).rgb * textureBlendRatio.w);
	}

	vec4 outColour = calcualteLightContrib( materialColour.rgb, fNormal.xyz, 
	                                        fVertWorldLocation.xyz, specularColour );
											
	pixelColour = outColour;
	
	// Copy alpha value from the diffuse.
	
	if ( ! bUseTextureAlpha )
	{
		pixelColour.a = diffuseColour.a;
	}
	else
	{	// True. Use alpha texture as alpha 
		// uniform sampler2D texAphaBlendSampler;
		pixelColour.a = texture( texAphaBlendSampler, fUVx2.xy ).r;
	}
	
	
	// Add some ambient light 
	// Just add a little bit of while light into the scene
	pixelColour.rgb += 0.10f; 
	// Bump up the brightness (dim projector)
	pixelColour.rgb *= 1.25f;

}






vec4 calcualteLightContrib( vec3 vertexMaterialColour, vec3 vertexNormal, 
                            vec3 vertexWorldPos, vec4 vertexSpecular )
{
	vec3 norm = normalize(vertexNormal);
	
	vec4 finalObjectColour = vec4( 0.0f, 0.0f, 0.0f, 1.0f );
	
	for ( int index = 0; index < NUMBEROFLIGHTS; index++ )
	{	
		// ********************************************************
		// is light "on"
		if ( theLights[index].param2.x == 0.0f )
		{	// it's off
			continue;
		}
		
		// Cast to an int (note with c'tor)
		int intLightType = int(theLights[index].param1.x);
		
		// We will do the directional light here... 
		// (BEFORE the attenuation, since sunlight has no attenuation, really)
		if ( intLightType == DIRECTIONAL_LIGHT_TYPE )		// = 2
		{
			// This is supposed to simulate sunlight. 
			// SO: 
			// -- There's ONLY direction, no position
			// -- Almost always, there's only 1 of these in a scene
			// Cheapest light to calculate. 

			vec3 lightContrib = theLights[index].diffuse.rgb;
			
			// Get the dot product of the light and normalize
			float dotProduct = dot( -theLights[index].direction.xyz,  
									   normalize(norm.xyz) );	// -1 to 1

			dotProduct = max( 0.0f, dotProduct );		// 0 to 1
		
			lightContrib *= dotProduct;		
			
			finalObjectColour.rgb += (vertexMaterialColour.rgb *           theLights[index].diffuse.rgb * lightContrib); 
									 //+ (materialSpecular.rgb * lightSpecularContrib.rgb);
			// NOTE: There isn't any attenuation, like with sunlight.
			// (This is part of the reason directional lights are fast to calculate)


			return finalObjectColour;		
		}
		
		// Assume it's a point light 
		// intLightType = 0
		
		// Contribution for this light
		vec3 vLightToVertex = theLights[index].position.xyz - vertexWorldPos.xyz;
		float distanceToLight = length(vLightToVertex);	
		vec3 lightVector = normalize(vLightToVertex);
		float dotProduct = dot(lightVector, vertexNormal.xyz);	 
		
		dotProduct = max( 0.0f, dotProduct );	
		
		vec3 lightDiffuseContrib = dotProduct * theLights[index].diffuse.rgb;
			

		// Specular 
		vec3 lightSpecularContrib = vec3(0.0f);
			
		vec3 reflectVector = reflect( -lightVector, normalize(norm.xyz) );

		// Get eye or view vector
		// The location of the vertex in the world to your eye
		vec3 eyeVector = normalize(eyeLocation.xyz - vertexWorldPos.xyz);

		// To simplify, we are NOT using the light specular value, just the object’s.
		float objectSpecularPower = vertexSpecular.w; 
		
//		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
//			                   * vertexSpecular.rgb;	//* theLights[lightIndex].Specular.rgb
		lightSpecularContrib = pow( max(0.0f, dot( eyeVector, reflectVector) ), objectSpecularPower )
			                   * theLights[index].specular.rgb;
							   
		// Attenuation
		float attenuation = 1.0f / 
				( theLights[index].atten.x + 										
				  theLights[index].atten.y * distanceToLight +						
				  theLights[index].atten.z * distanceToLight*distanceToLight );  	
				  
		// total light contribution is Diffuse + Specular
		lightDiffuseContrib *= attenuation;
		lightSpecularContrib *= attenuation;
		
		
		// But is it a spot light
		if ( intLightType == SPOT_LIGHT_TYPE )		// = 1
		{	
		

			// Yes, it's a spotlight
			// Calcualate light vector (light to vertex, in world)
			vec3 vertexToLight = vertexWorldPos.xyz - theLights[index].position.xyz;

			vertexToLight = normalize(vertexToLight);

			float currentLightRayAngle
					= dot( vertexToLight.xyz, theLights[index].direction.xyz );
					
			currentLightRayAngle = max(0.0f, currentLightRayAngle);

			//vec4 param1;	
			// x = lightType, y = inner angle, z = outer angle, w = TBD

			// Is this inside the cone? 
			float outerConeAngleCos = cos(radians(theLights[index].param1.z));
			float innerConeAngleCos = cos(radians(theLights[index].param1.y));
							
			// Is it completely outside of the spot?
			if ( currentLightRayAngle < outerConeAngleCos )
			{
				// Nope. so it's in the dark
				lightDiffuseContrib = vec3(0.0f, 0.0f, 0.0f);
				lightSpecularContrib = vec3(0.0f, 0.0f, 0.0f);
			}
			else if ( currentLightRayAngle < innerConeAngleCos )
			{
				// Angle is between the inner and outer cone
				// (this is called the penumbra of the spot light, by the way)
				// 
				// This blends the brightness from full brightness, near the inner cone
				//	to black, near the outter cone
				float penumbraRatio = (currentLightRayAngle - outerConeAngleCos) / 
									  (innerConeAngleCos - outerConeAngleCos);
									  
				lightDiffuseContrib *= penumbraRatio;
				lightSpecularContrib *= penumbraRatio;
			}
						
		}// if ( intLightType == 1 )
		
		
					
		finalObjectColour.rgb += (vertexMaterialColour.rgb * lightDiffuseContrib.rgb)
								  + (vertexSpecular.rgb  * lightSpecularContrib.rgb );

	}//for(intindex=0...
	
	finalObjectColour.a = 1.0f;
	
	return finalObjectColour;
}
