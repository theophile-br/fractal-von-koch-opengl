#shader vertex
#version 330 core
layout(location=0) in vec4 pos;

void main(){
      gl_Position = vec4(pos.x,pos.y, 0.0,1.0);
};


#shader fragment
#version 330 core
out vec4 color;

uniform vec4 u_color;

void main(){
    color = u_color;
};