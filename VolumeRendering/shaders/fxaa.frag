#version 420
layout(binding=0) uniform sampler2D colorBuffer;

uniform vec2 pixelSize;

in vec2     uvs;
out vec4 	oColor;

void main( void )
{
	float fxaaSpanMax	= 8.0;
	float fxaaReduceMul	= 1.0 / fxaaSpanMax;
	float fxaaReduceMin	= 1.0 / 128.0;

	vec3 rgbUL	= texture( colorBuffer, uvs + vec2( -1.0, -1.0 ) * pixelSize ).xyz;
	vec3 rgbUR	= texture( colorBuffer, uvs + vec2(  1.0, -1.0 ) * pixelSize ).xyz;
	vec3 rgbBL	= texture( colorBuffer, uvs + vec2( -1.0,  1.0 ) * pixelSize ).xyz;
	vec3 rgbBR	= texture( colorBuffer, uvs + vec2(  1.0,  1.0 ) * pixelSize ).xyz;
	vec3 rgbM	= texture( colorBuffer, uvs ).xyz;

	vec3 luma		= vec3( 0.299, 0.587, 0.114 );
	float lumaUL	= dot( rgbUL, luma );
	float lumaUR	= dot( rgbUR, luma );
	float lumaBL	= dot( rgbBL, luma );
	float lumaBR	= dot( rgbBR, luma );
	float lumaM		= dot( rgbM,  luma );
	
	float lumaMin = min( lumaM, min( min( lumaUL, lumaUR ), min( lumaBL, lumaBR ) ) );
	float lumaMax = max( lumaM, max( max( lumaUL, lumaUR ), max( lumaBL, lumaBR ) ) );
	
	vec2 dir = vec2( -( ( lumaUL + lumaUR ) - ( lumaBL + lumaBR ) ), ( lumaUL + lumaBL ) - ( lumaUR + lumaBR ) );
	
	float dirReduce	= max( ( lumaUL + lumaUR + lumaBL + lumaBR ) * ( 0.25 * fxaaReduceMul ), fxaaReduceMin );
	float rcpDirMin	= 1.0 / ( min ( abs( dir.x ), abs( dir.y ) ) + dirReduce );
	
	dir = min( vec2(  fxaaSpanMax,  fxaaSpanMax ),
		  max( vec2( -fxaaSpanMax, -fxaaSpanMax ),
		  dir * rcpDirMin ) ) * pixelSize;

	vec3 color0 = 0.5 * (
		texture( colorBuffer, uvs + dir * ( 1.0 / 3.0 - 0.5 ) ).rgb +
		texture( colorBuffer, uvs + dir * ( 2.0 / 3.0 - 0.5 ) ).rgb );
	vec3 color1 = color0 * 0.5 + 0.25 * (
		texture( colorBuffer, uvs + dir * -0.5 ).rgb +
		texture( colorBuffer, uvs + dir *  0.5 ).rgb );
	
	float lumaB = dot( color1, luma );

	vec4 color	= vec4( color1, 1.0 );

	if ( lumaB < lumaMin || lumaB > lumaMax ) 
    {
		color.rgb = color0;
	}

	oColor = color;
}