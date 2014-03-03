//
//  IMGUI.h
//  Stonefish
//
//  Created by Patryk Cieslak on 11/27/12.
//  Copyright (c) 2012 Patryk Cieslak. All rights reserved.
//

#ifndef __Stonefish_IMGUI__
#define __Stonefish_IMGUI__

#include "common.h"
#include "OpenGLPrinter.h"

//font
#define FONT_NAME "/Library/Fonts/Arial Narrow Bold.ttf"
#define FONT_SIZE 12

//interface colors
#define PANEL_BACKGROUND_COLOR 0
#define PANEL_TOP_BAR_COLOR 1
#define PANEL_LEFT_BAR_COLOR 2
#define PANEL_RIGHT_BAR_COLOR 3
#define PANEL_BORDER_COLOR 4
#define PANEL_TITLE_COLOR 5
#define BUTTON_NORMAL_COLOR 6
#define BUTTON_HOT_COLOR 7
#define BUTTON_ACTIVE_COLOR 8
#define BUTTON_BORDER_COLOR 9
#define BUTTON_TITLE_COLOR 10
#define SLIDER_BAR_EMPTY_COLOR 11
#define SLIDER_BAR_FILLED_COLOR 12
#define SLIDER_NORMAL_COLOR 13
#define SLIDER_HOT_COLOR 14
#define SLIDER_ACTIVE_COLOR 15
#define SLIDER_BORDER_COLOR 16
#define SLIDER_TITLE_COLOR 17
#define SLIDER_VALUE_COLOR 18
#define PROGRESSBAR_EMPTY_COLOR 19
#define PROGRESSBAR_FILLED_COLOR 20
#define PROGRESSBAR_BORDER_COLOR 21
#define PROGRESSBAR_TITLE_COLOR 22
#define PROGRESSBAR_VALUE_COLOR 23
#define CHECKBOX_NORMAL_COLOR 24
#define CHECKBOX_HOT_COLOR 25
#define CHECKBOX_ACTIVE_COLOR 26
#define CHECKBOX_BORDER_COLOR 27
#define CHECKBOX_TICK_COLOR 28
#define CHECKBOX_TITLE_COLOR 29
#define RADIOBUTTON_NORMAL_COLOR 30
#define RADIOBUTTON_HOT_COLOR 31
#define RADIOBUTTON_ACTIVE_COLOR 32
#define RADIOBUTTON_BORDER_COLOR 33
#define RADIOBUTTON_BUTTON_COLOR 34
#define RADIOBUTTON_TITLE_COLOR 35

struct ui_id
{
    int owner;
    int item;
    int index;
};

class IMGUI
{
public:
    //static
    static void DoLabel(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, GLfloat* color, const char* text);
    static void DoPanel(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title);
    static void DoProgressBar(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, double progress, const char* title);
    static bool DoButton(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat h, const char* title);
    static double DoSlider(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, GLfloat w, GLfloat sliderW, GLfloat sliderH, double min, double max, double value, const char* title);
    static bool DoCheckBox(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, bool value, const char* title);
    static bool DoRadioButton(IMGUI* UI, ui_id ID, GLfloat x, GLfloat y, bool value, const char* title);
    
    //non-static
    IMGUI();
    ~IMGUI();
    void SetRenderSize(int width, int height);
    ui_id getHot();
    ui_id getActive();
    void setHot(ui_id newHot);
    void setActive(ui_id newActive);
    bool isHot(ui_id ID);
    bool isActive(ui_id ID);
    void clearActive();
    void clearHot();
    int getWindowHeight();
    int getWindowWidth();
    
    //graphics
    void Init();
    void Begin();
    void End();
    bool MouseInRect(int x, int y, int w, int h);
    bool MouseIsDown(bool leftButton);
    int getMouseX();
    int getMouseY();
    
    void DrawPlainText(GLfloat x, GLfloat y, GLfloat* color, const char* text);
    GLfloat PlainTextLength(const char* text);
    
    //input handling
    void MouseDown(int x, int y, bool leftButton);
    void MouseUp(int x, int y, bool leftButton);
    void MouseMove(int x, int y);
    void KeyDown(SDL_Keycode key);
    void KeyUp(SDL_Keycode key);
    
private:
    int windowW,windowH;
    bool shaders;
    int mouseX, mouseY;
    bool mouseLeftDown, mouseRightDown;
    ui_id hot;
    ui_id active;
    OpenGLPrinter* plainPrinter;
};

#endif
