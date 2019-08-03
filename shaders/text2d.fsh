#version 330  
in vec2 UV1;
uniform vec4 textColor;
uniform sampler2D texSampler;

const float threshold = 0.2;
const float contrast = 20.;

out vec4 FragColor;



void main()
{

//маска из текстуры
 float mask = texture2D( texSampler, UV1).a;
 
 // use current drawing color
    vec4 clr=textColor;
	
	/*if( mask < 0.61 )  
	{
		clr=vec4(1.0)-clr;
		clr.a=1.0;
	}*/
	
 // perform simple thresholding
    if( mask < 0.5 )  
	{
		discard;
        //clr.a = 0.0;
	}
    else  
	{
        clr.a = 1.0;
	}
	

	
	// do some anti-aliasing
    clr.a *= smoothstep(0.5-threshold, 0.5+threshold, mask);
	// final color
    FragColor = clr; 
	//FragColor=vec4(1.0,0.0,0.0,1.0);
}