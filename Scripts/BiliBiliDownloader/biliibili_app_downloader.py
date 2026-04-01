#!/usr/bin/env python3
"""
WeChat Automation Script - Search, open chat and send message

Usage:
    python wechat_automation.py "video_name" "message"
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


def click_image(template_path, timeout=10, confidence=0.8, double_click=False, move=True):
    pos = wait_for_image(template_path, timeout, confidence)
    if pos:
        pyautogui.click(pos.x, pos.y)

        if move:
            pyautogui.moveTo(pos.x, pos.y, duration=0.2)

        if double_click:
            time.sleep(0.2)
            pyautogui.click(pos.x, pos.y)
        return True
    return False


def click_image_by_list(template_list, timeout=10, confidence=0.8, double_click=False, move=True):
    for template_path in template_list:
        pos = wait_for_image(template_path, timeout, confidence)
        if pos:
            print(f"Found {template_path} at {pos}")
            break

    if pos:
        pyautogui.click(pos.x, pos.y)

        if move:
            pyautogui.moveTo(pos.x, pos.y, duration=0.2)

        if double_click:
            time.sleep(0.2)
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
    parser = argparse.ArgumentParser(description="BiliBili Downloader")
    parser.add_argument("video_name", type=str, nargs="?", help="Video name to search")
    parser.add_argument("message", type=str, nargs="?", help="Message to send")
    parser.add_argument("--template-dir", type=str, default=".", help="Directory containing template images")
    parser.add_argument("--confidence", type=float, default=0.8, help="Image match confidence (0-1)")

    args = parser.parse_args()

    if not args.video_name or not args.message:
        parser.print_help()
        print("\nExample: python bilibili_automation.py \"张三\" \"你好\"")
        sys.exit(1)

    video_name = args.video_name
    message = args.message
    template_dir = args.template_dir
    confidence = args.confidence

    print(f"Searching for: {video_name}")
    print(f"Message: {message}")

    searchIcons = [f"{template_dir}/SearchInput1-1080.png", f"{template_dir}/SearchInput1-2160.png"]
    searchInputs = [f"{template_dir}/SearchInput2-1080.png", f"{template_dir}/SearchInput2-2160.png"]
    search_list = f"{template_dir}/bilibili_search_list.png"
    text_input = f"{template_dir}/bilibili_text_input.png"
    chat_input = f"{template_dir}/bilibili_chatinput.png"
    debug_png = f"{template_dir}/bilibili_debug.png"
    error_png = f"{template_dir}/bilibili_error.png"

    try:
        print("Step 1: Looking for search input...")
        if click_image_by_list(searchIcons, timeout=15, confidence=confidence, double_click=True):
            print("Search input found and clicked")
            time.sleep(0.5)

            if click_image_by_list(searchInputs, timeout=5, confidence=confidence, double_click=True):
                print(f"Step 2: Typing contact name: {video_name}")
                pyautogui.write(video_name, interval=0.2)
                #pyperclip.copy(video_name)
                #pyautogui.hotkey('command', 'v')
                time.sleep(0.2)
                pyautogui.press('enter')
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
            print("Search icon not found. Make sure bilibili is open and templates are correct.")
            print(f"Expected template: {searchIcons}")
            SaveScreenshot(debug_png)

    except Exception as e:


        print(f"Error: {e}")
        SaveScreenshot(error_png)
        raise

    print("\nDone!")


if __name__ == "__main__":
    main()
