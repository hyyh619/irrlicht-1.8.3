#!/usr/bin/env python3
"""
WeChat Automation Script - Search, open chat and send message

Usage:
    python wechat_automation.py "contact_name" "message"
"""

import sys
import argparse
import time
import pyautogui
import subprocess


pyautogui.PAUSE = 1.0
pyautogui.FAILSAFE = True


def wait_for_image(template_path, timeout=10, confidence=0.8):
    start = time.time()
    while time.time() - start < timeout:
        try:
            pos = pyautogui.locateCenterOnScreen(template_path, confidence=confidence)
            if pos:
                return pos
        except Exception:
            pass
        time.sleep(0.5)
    return None


def click_image(template_path, timeout=10, confidence=0.8):
    pos = wait_for_image(template_path, timeout, confidence)
    if pos:
        pyautogui.click(pos.x, pos.y)
        return True
    return False


def activate_wechat():
    try:
        subprocess.run(['powershell', '-Command', 'Start-Process', 'WeChat'], check=False)
        time.sleep(3)
    except Exception:
        pass


def main():
    parser = argparse.ArgumentParser(description="WeChat Automation")
    parser.add_argument("contact_name", type=str, nargs="?", help="Contact name to search")
    parser.add_argument("message", type=str, nargs="?", help="Message to send")
    parser.add_argument("--template-dir", type=str, default=".", help="Directory containing template images")
    parser.add_argument("--confidence", type=float, default=0.8, help="Image match confidence (0-1)")
    
    args = parser.parse_args()
    
    if not args.contact_name or not args.message:
        parser.print_help()
        print("\nExample: python wechat_automation.py \"张三\" \"你好\"")
        sys.exit(1)
    
    contact_name = args.contact_name
    message = args.message
    template_dir = args.template_dir
    confidence = args.confidence
    
    print(f"Searching for: {contact_name}")
    print(f"Message: {message}")
    
    search_icon = f"{template_dir}/search_icon.png"
    chat_input = f"{template_dir}/chat_input.png"
    
    try:
        activate_wechat()
        
        print("Step 1: Looking for search icon...")
        if click_image(search_icon, timeout=15, confidence=confidence):
            print("  Search icon found and clicked")
            time.sleep(0.5)
            
            print(f"Step 2: Typing contact name: {contact_name}")
            pyautogui.write(contact_name, interval=0.1)
            time.sleep(1)
            
            print("Step 3: Clicking first result...")
            pyautogui.press('enter')
            time.sleep(1)
            
            print(f"Step 4: Typing message: {message}")
            pyautogui.write(message, interval=0.05)
            time.sleep(0.5)
            
            print("Step 5: Sending message...")
            pyautogui.press('enter')
            
            print("Message sent successfully!")
        else:
            print("Search icon not found. Make sure WeChat is open and templates are correct.")
            print(f"Expected template: {search_icon}")
            screenshot_path = "wechat_debug.png"
            pyautogui.screenshot(screenshot_path)
            print(f"Screenshot saved to {screenshot_path}")
            
    except Exception as e:
        print(f"Error: {e}")
        pyautogui.screenshot("wechat_error.png")
        print("Screenshot saved to wechat_error.png")
        raise
    
    print("\nDone!")


if __name__ == "__main__":
    main()
