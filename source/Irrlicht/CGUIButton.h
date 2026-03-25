// Copyright (C) 2002-2012 Nikolaus Gebhardt
// This file is part of the "Irrlicht Engine".
// For conditions of distribution and use, see copyright notice in irrlicht.h

#ifndef __C_GUI_BUTTON_H_INCLUDED__
#define __C_GUI_BUTTON_H_INCLUDED__

#include "IrrCompileConfig.h"
#ifdef _IRR_COMPILE_WITH_GUI_

#include "IGUIButton.h"
#include "IGUISpriteBank.h"
#include "SColor.h"

namespace irr
{
namespace gui
{

/**
 * @brief Button GUI element
 * 
 * A clickable button control with support for images,
 * sprites, push-button behavior, and customization.
 */
class CGUIButton : public IGUIButton
{
public:

    /**
     * @brief Constructor
     * @param environment GUI environment
     * @param parent Parent element
     * @param id Element ID
     * @param rectangle Button rectangle
     * @param noclip Disable clipping
     */
    CGUIButton(IGUIEnvironment *environment, IGUIElement *parent,
               s32 id, core::rect<s32> rectangle, bool noclip = false);

    /**
     * @brief Destructor
     */
    virtual ~CGUIButton();

    /**
     * @brief Handle input events
     * @param event Input event
     * @return true if event was handled
     */
    virtual bool OnEvent(const SEvent &event);

    /**
     * @brief Draw the button
     */
    virtual void draw();

    /**
     * @brief Set override font
     * @param font Font to use (0 = use skin font)
     */
    virtual void setOverrideFont(IGUIFont *font = 0);

    /**
     * @brief Get override font
     * @return Override font or 0
     */
    virtual IGUIFont* getOverrideFont() const;

    /**
     * @brief Get active font
     * @return Font currently being used
     */
    virtual IGUIFont* getActiveFont() const;

    /**
     * @brief Set button image
     * @param image Texture to display
     */
    virtual void setImage(video::ITexture *image = 0);

    /**
     * @brief Set button image with position
     * @param image Texture to display
     * @param pos Image rectangle
     */
    virtual void setImage(video::ITexture *image, const core::rect<s32> &pos);

    /**
     * @brief Set pressed state image
     * @param image Texture to display when pressed
     */
    virtual void setPressedImage(video::ITexture *image = 0);

    /**
     * @brief Set pressed state image with position
     * @param image Texture to display when pressed
     * @param pos Image rectangle
     */
    virtual void setPressedImage(video::ITexture *image, const core::rect<s32> &pos);

    /**
     * @brief Set sprite bank
     * @param bank Sprite bank to use
     */
    virtual void setSpriteBank(IGUISpriteBank *bank = 0);

    /**
     * @brief Set button sprite
     * @param state Button state
     * @param index Sprite index in bank
     * @param color Sprite color
     * @param loop Loop animation
     */
    virtual void setSprite(EGUI_BUTTON_STATE state, s32 index,
                           video::SColor color = video::SColor(255, 255, 255, 255), bool loop = false);

    /**
     * @brief Set push button mode
     * @param isPushButton true for toggle behavior
     */
    virtual void setIsPushButton(bool isPushButton = true);

    /**
     * @brief Check if push button
     * @return true if push button mode
     */
    virtual bool isPushButton() const;

    /**
     * @brief Set pressed state
     * @param pressed Pressed state
     */
    virtual void setPressed(bool pressed = true);

    /**
     * @brief Check if pressed
     * @return true if currently pressed
     */
    virtual bool isPressed() const;

    /**
     * @brief Set border drawing
     * @param border Draw border flag
     */
    virtual void setDrawBorder(bool border = true);

    /**
     * @brief Check if border is drawn
     * @return true if border is drawn
     */
    virtual bool isDrawingBorder() const;

    /**
     * @brief Set alpha channel usage
     * @param useAlphaChannel Use alpha channel
     */
    virtual void setUseAlphaChannel(bool useAlphaChannel = true);

    /**
     * @brief Check alpha channel usage
     * @return true if alpha channel is used
     */
    virtual bool isAlphaChannelUsed() const;

    /**
     * @brief Set image scaling
     * @param scaleImage Scale image to fit button
     */
    virtual void setScaleImage(bool scaleImage = true);

    /**
     * @brief Check if image scaling
     * @return true if images are scaled
     */
    virtual bool isScalingImage() const;

    /**
     * @brief Serialize element attributes
     * @param out Output attributes
     * @param options Read/write options
     */
    virtual void serializeAttributes(io::IAttributes *out, io::SAttributeReadWriteOptions *options) const;

    /**
     * @brief Deserialize element attributes
     * @param in Input attributes
     * @param options Read/write options
     */
    virtual void deserializeAttributes(io::IAttributes *in, io::SAttributeReadWriteOptions *options);

private:

    /**
     * @brief Button sprite data
     */
    struct ButtonSprite
    {
        s32           Index;         ///< Sprite index
        video::SColor Color;        ///< Sprite color
        bool          Loop;         ///< Animation loop
    };

    ButtonSprite ButtonSprites[EGBS_COUNT];  ///< Sprites for each state

    IGUISpriteBank *SpriteBank;  ///< Sprite bank
    IGUIFont       *OverrideFont; ///< Override font

    video::ITexture *Image;         ///< Normal image
    video::ITexture *PressedImage;  ///< Pressed image

    core::rect<s32> ImageRect;         ///< Normal image rect
    core::rect<s32> PressedImageRect;  ///< Pressed image rect

    u32 ClickTime, HoverTime, FocusTime;  ///< Timing for events

    bool IsPushButton;    ///< Push button mode
    bool Pressed;         ///< Pressed state
    bool UseAlphaChannel; ///< Use alpha channel
    bool DrawBorder;      ///< Draw border
    bool ScaleImage;      ///< Scale image
};
}   // end namespace gui
} // end namespace irr
#endif // _IRR_COMPILE_WITH_GUI_
#endif // __C_GUI_BUTTON_H_INCLUDED__