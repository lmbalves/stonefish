#version 120

void main(void)
{
	gl_TexCoord[0] = vec4(gl_Vertex.zw, 0, 0);
	gl_Position = vec4(gl_Vertex.xy, 0, 1.0);
}
