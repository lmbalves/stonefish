//
//  OpenGLPipeline.h
//  Stonefish
//
//  Created by Patryk Cieslak on 30/03/2014.
//  Copyright (c) 2014-2018 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_OpenGLPipeline__
#define __Stonefish_OpenGLPipeline__

#include <SDL2/SDL_thread.h>
#include "StonefishCommon.h"
#include "graphics/OpenGLDataStructs.h"

namespace sf
{
    class SimulationManager;
    class OpenGLContent;
    class OpenGLCamera;
    
    //! A class implementing the OpenGL rendering pipeline.
    class OpenGLPipeline
    {
    public:
        //! A constructor.
        /*!
         \param s a structure containing render settings
         */
        OpenGLPipeline(RenderSettings s, HelperSettings h);
        
        //! A destructor.
        ~OpenGLPipeline();
        
        //! A method that constitutes the main rendering pipeline.
        /*!
         \param sim a pointer to the simulation manager
         */
        void Render(SimulationManager* sim);
        
        //! A method to add renderable objects to the rendering queue.
        /*!
         \param r a renderable object
         */
        void AddToDrawingQueue(Renderable r);
        
        //! A method that draws all normal objects.
        void DrawObjects();
        
        //! A method that blits the screen FBO to the main framebuffer.
        void DrawDisplay();
        
        //! A method that informs if the drawing queue is empty.
        bool isDrawingQueueEmpty();
        
        //! A method to get mutex of the drawing queue for thread safeness.
        SDL_mutex* getDrawingQueueMutex();
        
        //! A method returning a copy of the render settings.
        RenderSettings getRenderSettings() const;
        
        //! A method returning a reference to the helper object settings.
        HelperSettings& getHelperSettings();

        //! A method returning the screen texture, used for generating GUI background.
        GLuint getScreenTexture();
        
        //! A method returning a pointer to the OpenGL content manager.
        OpenGLContent* getContent();
        
    private:
        void PerformDrawingQueueCopy();
        void DrawHelpers();
        
        RenderSettings rSettings;
        HelperSettings hSettings;
        std::vector<Renderable> drawingQueue;
        std::vector<Renderable> drawingQueueCopy;
        SDL_mutex* drawingQueueMutex;
        GLuint screenFBO;
        GLuint screenTex;
        OpenGLContent* content;
    };
}

#endif