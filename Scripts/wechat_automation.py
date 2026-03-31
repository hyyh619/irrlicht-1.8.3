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
import pyperclip
import subprocess
from screeninfo import get_monitors


pyautogui.PAUSE = 1.0
pyautogui.FAILSAFE = True
g_monitors = get_monitors()


def ScreenshotMonitor(m):
    # 获取该显示器的几何参数：x, y, width, height
    region = (m.x, m.y, m.width, m.height)
    return pyautogui.screenshot(region=region)


def SaveScreenshot(filename):
    global g_monitors
    monitors = g_monitors

    index = 0
    for m in monitors:
        print(f"Monitor {index}: {m.width}x{m.height} at ({m.x}, {m.y})") 
        index += 1
        img = ScreenshotMonitor(m)
        monitorFile = filename.replace(".png", f"_monitor{index}.png")
        img.save(monitorFile)
        print(f"Screenshot saved to {monitorFile}")
    return


def wait_for_image(template_path, timeout=10, confidence=0.8):
    global g_monitors
    monitors = g_monitors

    actConf = confidence
    start = time.time()
    while time.time() - start < timeout:
        for m in monitors:
            try:
                print(f"Searching for {template_path} on monitor ({m.x}, {m.y}, {m.width}, {m.height}) with confidence {actConf:.2f}")
                pos = pyautogui.locateCenterOnScreen(
                    template_path, confidence=actConf, region=(m.x, m.y, m.width, m.height), grayscale=True)
                if pos:
                    print(f"Found {template_path} at {pos} on monitor ({m.x}, {m.y})")
                    return pos
            except Exception:
                pass

        # actConf -= 0.01
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
    
    search_icon = f"{template_dir}/wechat_search_text3.png"
    search_list = f"{template_dir}/wechat_search_list.png"
    text_input = f"{template_dir}/wechat_text_input.png"
    chat_input = f"{template_dir}/wechat_chatinput.png"
    debug_png = f"{template_dir}/wechat_debug.png"
    error_png = f"{template_dir}/wechat_error.png"
    
    try:
        # activate_wechat()
        
        print("Step 1: Looking for search icon...")
        if click_image(search_icon, timeout=15, confidence=confidence):
            print("  Search icon found and clicked")
            time.sleep(0.5)
            
            print(f"Step 2: Typing contact name: {contact_name}")
            # pyautogui.write(contact_name, interval=0.1)
            pyperclip.copy(contact_name)
            pyautogui.hotkey('command', 'v')
            time.sleep(1)

            if click_image(search_list, timeout=5, confidence=confidence):
                print("Step 3: Search result found and clicked")

                if click_image(text_input, timeout=5, confidence=confidence):
                    print(f"Step 4: Typing message: {message}")
                    pyperclip.copy(message)
                    pyautogui.hotkey('command', 'v')
                    pyautogui.press('enter')
                    time.sleep(1)
                    print("Message sent successfully!")
            else:
                print("Step 3: Cannot find the user.")
        else:
            print("Search icon not found. Make sure WeChat is open and templates are correct.")
            print(f"Expected template: {search_icon}")
            SaveScreenshot(debug_png)

    except Exception as e:

        
        print(f"Error: {e}")
        SaveScreenshot(error_png)
        raise
    
    print("\nDone!")


if __name__ == "__main__":
    main()
