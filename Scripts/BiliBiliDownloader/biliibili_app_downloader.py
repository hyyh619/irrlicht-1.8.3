#!/usr/bin/env python3
"""
WeChat Automation Script - Search, open chat and send message

Usage:
    python wechat_automation.py "video_name" "message"
"""

import os
#import cv2
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

state_init              = 0
state_search_icon_right = 1
state_search_input      = 2
state_search_result     = 3
state_unknown           = 99 

def apple_script_paste():
    # 直接让 macOS 系统进程执行“按下 command + v”
    cmd = 'osascript -e "tell application \\"System Events\\" to keystroke \\"v\\" using command down"'
    os.system(cmd)
    return


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
                posLeft = pyautogui.locateOnScreen(
                    template_path, confidence=actConf, region=(m.x, m.y, m.width, m.height), grayscale=True)
                pos = pyautogui.locateCenterOnScreen(
                    template_path, confidence=actConf, region=(m.x, m.y, m.width, m.height), grayscale=True)
                if pos:
                    print(f"Found {template_path} at {pos} on monitor ({m.x}, {m.y})")
                    return pos, posLeft
            except Exception:
                pass

        # actConf -= 0.01
        time.sleep(0.5)
    return None, None


def wait_for_image_by_list(template_list, timeout=10, confidence=0.8):
    global g_monitors
    monitors = g_monitors

    actConf = confidence
    start = time.time()
    while time.time() - start < timeout:
        for m in monitors:
            for template_path in template_list:
                try:
                    print(f"Searching for {template_path} on monitor ({m.x}, {m.y}, {m.width}, {m.height}) with confidence {actConf:.2f}")
                    posLeft = pyautogui.locateOnScreen(
                        template_path, confidence=actConf, region=(m.x, m.y, m.width, m.height), grayscale=True)
                    pos = pyautogui.locateCenterOnScreen(
                        template_path, confidence=actConf, region=(m.x, m.y, m.width, m.height), grayscale=True)
                    if pos:
                        print(f"Found {template_path} at {pos} on monitor ({m.x}, {m.y})")
                        return pos, posLeft
                except Exception:
                    pass

        # actConf -= 0.01
        time.sleep(0.5)
    return None, None


def click_image(template_path, timeout=10, confidence=0.8, double_click=False, move=True):
    pos, posLeft = wait_for_image(template_path, timeout, confidence)
    if pos:
        pyautogui.click(pos.x, pos.y)

        if move:
            pyautogui.moveTo(posLeft.left+5, posLeft.Top+5, duration=0.2)
            time.sleep(0.2)

        if double_click:
            time.sleep(0.2)
            pyautogui.click(pos.x, pos.y)
        return True
    return False


def click_image_by_list(template_list, timeout=10, confidence=0.8, double_click=False, move=True):
    for template_path in template_list:
        pos, posLeft = wait_for_image(template_path, timeout, confidence)
        if pos:
            print(f"Found {template_path} at {pos}")
            break

    if pos:
        pyautogui.click(pos.x, pos.y)
        time.sleep(0.2)

        if move:
            pyautogui.moveTo(posLeft.left+5, posLeft.top+5, duration=0.2)
            time.sleep(0.2)

        if double_click:
            time.sleep(0.2)
            pyautogui.click(pos.x, pos.y)
        return True
    return False


def move_image_by_list(template_list, timeout=10, confidence=0.8):
    for template_path in template_list:
        pos, posLeft = wait_for_image(template_path, timeout, confidence)
        if pos:
            print(f"Found {template_path} at {pos}")
            break

    if pos:
        pyautogui.moveTo(posLeft.left+5, posLeft.top+5, duration=0.2)
        time.sleep(0.2)
        return True

    return False


def activate_wechat():
    try:
        subprocess.run(['powershell', '-Command', 'Start-Process', 'WeChat'], check=False)
        time.sleep(3)
    except Exception:
        pass
    
    
def CheckAndProcessSearchIcon(searchIcons, confidence, state):
    pos = wait_for_image_by_list(searchIcons, timeout=2, confidence=confidence)
    if pos:
        print("Step 1: Looking for search input...")
        if click_image_by_list(searchIcons, timeout=15, confidence=confidence, double_click=True):
            print("Search input found and clicked")
            time.sleep(0.5)

            print("Current state: Search-Right-Icon")
            return state_search_icon_right

    # reture default state
    return state


def CheckAndProcessSearchInput(searchInputs, searchInputMoves, video_name, confidence, state):
    pos = wait_for_image_by_list(searchInputs, timeout=2, confidence=confidence)
    if pos:
        print(f"Step 2: Typing video name: {video_name}")
        if click_image_by_list(searchInputs, timeout=5, confidence=confidence, double_click=True):
            # Move cursor
            move_image_by_list(searchInputMoves, timeout=5, confidence=confidence)
            time.sleep(0.2)

            #pyautogui.write(video_name, interval=0.2)
            #pyautogui.write(" ", interval=0.2)
            #time.sleep(5.2)
            pyperclip.copy(video_name)
            print(f"Copy video name: {video_name}")
            time.sleep(0.3)

            """
            # 立即读取出来打印
            current_clipboard = pyperclip.paste()

            if current_clipboard == video_name:
                print("剪贴板写入成功，问题出在下一步的 hotkey('command', 'v')")
            else:
                print("剪贴板写入失败，请检查 pyperclip 安装情况")
            """

            """
            There are two methods from pyautogui to copy/paste content.
            But the both are useless for bilibili application. 
            1. Using hotKey directly. (This method is verified in wechat. it works well.)
            pyautogui.hotkey('command', 'v')
            2. Using keyDown/Press/keyUp to simulate human operation.
            pyautogui.keyDown('command')
            time.sleep(0.1)
            pyautogui.press('v')
            time.sleep(0.1)
            pyautogui.keyUp('command')
            time.sleep(0.2)

            Therefore, we have to use apple script paste
            """
            apple_script_paste()

            pyautogui.press('enter')
            time.sleep(1)

            print("Current state: Search-Input")
            return state_search_input

    # reture default state
    return state


def CheckAndProcessSearchResult(searchResults, confidence, state):
    pos, posLeft = wait_for_image_by_list(searchResults, timeout=2, confidence=confidence)
    if pos:
        print("Step 3: click results...")
        
        # Move pos to the first video result
        pyautogui.click(posLeft.left + posLeft.width, posLeft.top + posLeft.height + 20)
        time.sleep(0.5)

        print("Current state: state_search_result")
        return state_search_result

    # reture default state
    return state


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
    searchInputMoves = [f"{template_dir}/SearchInputMoveDown-1080.png", f"{template_dir}/SearchInputMoveDown-2160.png"]
    searchResults = [f"{template_dir}/SearchResult-1080.png", f"{template_dir}/SearchResult-2160.png"]
    debug_png = f"{template_dir}/bilibili_debug.png"

    # Set default state
    state = state_init

    # Count the time and break the loop if it takes too long
    counter = 0
    
    while(True):
        # Check the current states
        state = CheckAndProcessSearchIcon(searchIcons, confidence, state)
        state = CheckAndProcessSearchInput(searchInputs, searchInputMoves, video_name, confidence, state)
        state = CheckAndProcessSearchResult(searchResults, confidence, state)

        if state == state_unknown or state == state_search_result:
            SaveScreenshot(debug_png)
            print(f"The last state is {state}, break the loop.")
            break
        
        if counter == 10:
            print("Timeout! Current state: ", state)
            SaveScreenshot(debug_png)
            print(f"Debug screenshot saved to {debug_png}")
            break

    print("\nDone!")


if __name__ == "__main__":
    main()
