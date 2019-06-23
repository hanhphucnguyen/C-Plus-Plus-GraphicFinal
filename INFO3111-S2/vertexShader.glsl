#version 420

//uniform mat4 MVP;
uniform mat4 matModel;		// "m"
uniform mat4 matView;		// "v"
uniform mat4 matProj;		// "p"
uniform mat4 matModelInvTrans;


in vec4 vCol;	
in vec4 vPos;		
in vec4 vNormal;
in vec4 vUVx2;		// NEW. We'll only use x and y

out vec4 fColour;		
out vec4 fNormal;
out vec4 fVertWorldLocation;
out vec4 fUVx2;		// NEW to the fragment shader

uniform sampler2D heightMapTexture;
uniform bool bUseMapTexture;
uniform vec2 scootchTexture;

void main()
{
	// mvp = p * v * m; 
	mat4 MVP = matProj * matView * matModel;
	vec4 curVertex = vPos;
	
	// Adjust if it's a height map
	if ( bUseMapTexture )
	{
		// Sample the texture and shift the vertex up in the y
		// Since it's black and white, we'll only sample the red (or blue or green
		
		vec2 finalUV;
		finalUV.xy = vUVx2.xy;
		
		finalUV.xy += scootchTexture.xy;
		
		float heightAdjust0to1 = texture( heightMapTexture, finalUV.xy ).r;
		
		const float SCALE = 25.0f;
				
		curVertex.y += ( heightAdjust0to1 * SCALE );	
	}
	
							 	
    gl_Position = MVP * curVertex;
	
	fVertWorldLocation = matModel * curVertex;
	
	fNormal = matModelInvTrans * vNormal;
	
    fColour = vCol;
	fUVx2 = vUVx2;
}



