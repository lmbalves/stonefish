//
//  Console.cpp
//  Stonefish
//
//  Created by Patryk Cieslak on 24/05/2014.
//  Copyright (c) 2014-2017 Patryk Cieslak. All rights reserved.
//

#include "Console.h"
#include "SystemUtil.hpp"
#include "stb_image.h"
#include "IMGUI.h"

Console* Console::instance = NULL;

Console::Console()
{
	windowW = 800;
    windowH = 600;
    ready = false;
	consoleVAO = 0;
	texQuadVBO = 0;
	texQuadShader = NULL;
	linesMutex = SDL_CreateMutex();
}

Console::~Console()
{
    lines.clear();
    delete printer;
    if(logoTexture > 0)
        glDeleteTextures(1, &logoTexture);
	if(consoleVAO > 0)
		glDeleteVertexArrays(1, &consoleVAO);
	if(texQuadVBO > 0)
		glDeleteBuffers(1, &texQuadVBO);
	if(texQuadShader != NULL)
		delete texQuadShader;
    SDL_DestroyMutex(linesMutex);
}

void Console::Init(GLuint w, GLuint h)
{
	if(ready)
		return;
		
	printer = new OpenGLPrinter(FONT_NAME, FONT_SIZE);
    OpenGLPrinter::SetWindowSize(w, h);
	scrollOffset = 0;
    scrollVelocity = 0;
	windowW = w;
	windowH = h;

    //Load logo texture - can't use material class because it writes to the console
    char path[1024];
    int width, height, channels;
    GetDataPath(path, 1024-32);
    strcat(path, "logo_color_64.png");
    
    // Allocate image; fail out on error
    unsigned char* dataBuffer = stbi_load(path, &width, &height, &channels, 4);
    if(dataBuffer != NULL)
    {
        // Allocate an OpenGL texture
        glGenTextures(1, &logoTexture);
        glBindTexture(GL_TEXTURE_2D, logoTexture);
        // Upload texture to memory
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, dataBuffer);
        // Set certain properties of texture
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Wrap texture around
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        // Release internal buffer
        stbi_image_free(dataBuffer);
    }
    else
        logoTexture = 0;
		
	glGenVertexArrays(1, &consoleVAO);
	glBindVertexArray(consoleVAO);
	glEnableVertexAttribArray(0);
	glBindVertexArray(0);
		
	GLfloat saqData[4][4] = {{-1.f, -1.f, 0.f, 0.f},
							 { 1.f, -1.f, 1.f, 0.f},
							 {-1.f,  1.f, 0.f, 1.f},
							 { 1.f,  1.f, 1.f, 1.f}};
	
	glGenBuffers(1, &texQuadVBO); 
	glBindBuffer(GL_ARRAY_BUFFER, texQuadVBO); 
	glBufferData(GL_ARRAY_BUFFER, sizeof(saqData), saqData, GL_STATIC_DRAW); 
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	
	texQuadShader = new GLSLShader("texQuad.frag","texQuad.vert");
	texQuadShader->AddUniform("rect", ParameterType::VEC4);
	texQuadShader->AddUniform("tex", ParameterType::INT);
	texQuadShader->AddUniform("color", ParameterType::VEC4);
	
	ready = true;
}

void Console::Scroll(GLfloat amount)
{
    scrollOffset += 1.f * amount;
    scrollVelocity = 25.f * amount;
}

void Console::ResetScroll()
{
    scrollOffset = 0.f;
    scrollVelocity = 0.f;
}

void Console::Render(bool overlay, GLfloat dt)
{
    if(lines.size() == 0)
        return;
    
    //Calculate visible lines range
    long int maxVisibleLines = (long int)floorf((GLfloat)windowH/(GLfloat)(FONT_SIZE + 5)) + 1;
    long int linesCount = lines.size();
    long int visibleLines = maxVisibleLines;
    long int scrolledLines = 0;
    
	GLfloat logoSize = 64.f;
    GLfloat logoMargin = 10.f;
	glm::vec4 info(1.f, 1.f, 1.f, 1.f);
	glm::vec4 warning(1.f, 1.f, 0.f, 1.f);
	glm::vec4 error(1.f, 0.f, 0.f, 1.f);
	glm::vec4 colors[3] = {info, warning, error};
    
    if(linesCount < maxVisibleLines) //there is enough space to display all lines
    {
        scrollOffset = 0.f;
        scrollVelocity = 0.f;
        visibleLines = linesCount;
    }
    else //there is more lines than can appear on screen at once
    {
        scrolledLines = (long int)floorf(-scrollOffset /(GLfloat)(FONT_SIZE + 5));
        scrolledLines = scrolledLines < 0 ? 0 : scrolledLines;
        if(linesCount < scrolledLines + visibleLines)
            visibleLines = linesCount - scrolledLines;

        if(scrollVelocity != 0.f) //velocity damping (momentum effect)
        {
            scrollOffset += scrollVelocity * dt;
            GLfloat dampingAcc = -scrollVelocity * 3.f;
            scrollVelocity += dampingAcc * dt;
        }
        
        //springy effect
        if(scrollOffset > 0.f) //the list is scrolled up too much
            scrollVelocity -= scrollOffset * 5.f * dt;
        else if(visibleLines < (maxVisibleLines - 1)) //the list is scrolled down too much
            scrollVelocity += (1.f - (GLfloat)visibleLines/(GLfloat)(maxVisibleLines - 1)) * (GLfloat)windowH * 5.f * dt;
    }
    
    //Setup viewport and ortho
    glScissor(0, 0, windowW, windowH);
    glViewport(0, 0, windowW, windowH);
	
	glDisable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	if(overlay)
	{
		texQuadShader->Use();
		texQuadShader->SetUniform("tex", 0);
		texQuadShader->SetUniform("color",  glm::vec4(0.3f,0.3f,0.3f,1.f));
		texQuadShader->SetUniform("rect", glm::vec4(0, 0, 1.f, 1.f));
		
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, IMGUI::getInstance()->getTranslucentTexture());
		
		glBindVertexArray(consoleVAO);
		
		glBindBuffer(GL_ARRAY_BUFFER, texQuadVBO); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		texQuadShader->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
		texQuadShader->SetUniform("rect", glm::vec4((windowW - logoSize - logoMargin)/(GLfloat)windowW, 1.f - (logoMargin+logoSize)/(GLfloat)windowH, logoSize/(GLfloat)windowW, logoSize/(GLfloat)windowH));
		
		glBindTexture(GL_TEXTURE_2D, logoTexture);
		
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glUseProgram(0);
		glBindTexture(GL_TEXTURE_2D, 0);
	
		//Text rendering
		for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
		{
			ConsoleMessage* msg = &lines[linesCount-1-i];
			printer->Print(colors[msg->type], 10.f, scrollOffset + 10.f + i * (FONT_SIZE + 5), FONT_SIZE, msg->text.c_str());
		}
		
		glBindVertexArray(0);
	}
	else //During loading of resources (displaying in second thread -> no VAO sharing)
	{
		texQuadShader->Use();
		texQuadShader->SetUniform("tex", 0);
		texQuadShader->SetUniform("color",  glm::vec4(1.f,1.f,1.f,1.f));
		texQuadShader->SetUniform("rect", glm::vec4((windowW - logoSize - logoMargin)/(GLfloat)windowW, 1.f - (logoMargin+logoSize)/(GLfloat)windowH, logoSize/(GLfloat)windowW, logoSize/(GLfloat)windowH));
		
		glActiveTexture(GL_TEXTURE0);
		glEnable(GL_TEXTURE_2D);
		glBindTexture(GL_TEXTURE_2D, logoTexture);
		
		glBindBuffer(GL_ARRAY_BUFFER, texQuadVBO); 
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		
 		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		
		glBindTexture(GL_TEXTURE_2D, 0);
		glUseProgram(0);
		
		//Text rendering
		for(long int i = scrolledLines; i < scrolledLines + visibleLines; i++)
		{
			ConsoleMessage* msg = &lines[linesCount-1-i];
			printer->Print(colors[msg->type], 10.f, scrollOffset + 10.f + i * (FONT_SIZE + 5), FONT_SIZE, msg->text.c_str());
		}
	}
}

void Console::Print(int messageType, const char *format, ...)
{
	va_list args;
    char buffer[4096];
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
	
#ifdef DEBUG
    printf("%s\n", buffer);
#endif
	
	if(!ready)
		return;
	
    ConsoleMessage msg;
    msg.type = messageType;
    msg.text = std::string(buffer);
    SDL_LockMutex(linesMutex);
    lines.push_back(msg);
    SDL_UnlockMutex(linesMutex);
}

void Console::Clear()
{
    SDL_LockMutex(linesMutex);
    lines.clear();
    SDL_UnlockMutex(linesMutex);
}

SDL_mutex* Console::getLinesMutex()
{
    return linesMutex;
}

Console* Console::getInstance()
{
    if(instance == NULL)
        instance = new Console();
    
    return instance;
}