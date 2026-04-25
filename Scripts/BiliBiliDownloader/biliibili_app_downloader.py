#!/usr/bin/env python3
"""
WeChat Automation Script - Search, open chat and send message

Usage:
    python wechat_automation.py "video_name" "message"
"""

import os
import cv2
import sys
import argparse
import time
import pyautogui
import pyperclip
import subprocess
from screeninfo import get_monitors
from paddleocr import PaddleOCR
import pyautogui
import numpy as np

pyautogui.PAUSE = 1.0
pyautogui.FAILSAFE = True
g_monitors = get_monitors()

state_init              = 0
state_search_icon_right = 1
state_search_input      = 2
state_search_result     = 3
state_cache_video       = 4
state_download_video    = 5
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
                # print(f"Searching for {template_path} on monitor ({m.x}, {m.y}, {m.width}, {m.height}) with confidence {actConf:.2f}")
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
                    # print(f"Searching for {template_path} on monitor ({m.x}, {m.y}, {m.width}, {m.height}) with confidence {actConf:.2f}")
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
            #print(f"Found {template_path} at {pos}")
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


def CheckAndProcessCacheVideo(cacheVideos, confidence, state):
    pos, posLeft = wait_for_image_by_list(cacheVideos, timeout=2, confidence=confidence)
    if pos:
        print("Step 4: Caching video...")

        # before click, we need to stop playing the video.
        time.sleep(5)
        pyautogui.press('space')
        time.sleep(0.1)
        print("Stop playing video...")

        # Move pos to the first video result
        pyautogui.move(pos.x, pos.y)

        if click_image_by_list(cacheVideos, timeout=15, confidence=confidence, double_click=False):
            time.sleep(0.5)

            print("Current state: state_cache_video")
            return state_cache_video

    # reture default state
    return state


def CheckAndProcessDownloadVideo(downloadVideos, confidence, state):
    pos, posLeft = wait_for_image_by_list(downloadVideos, timeout=5, confidence=confidence)
    if pos:
        print("Step 5: Downloading video...")

        if click_image_by_list(downloadVideos, timeout=15, confidence=confidence, double_click=False):
            time.sleep(0.5)

            print("Current state: state_download_video")
            return state_download_video

    # reture default state
    return state


def ParseArgs():
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

    return args


def GetDataFromArgs():
    args = ParseArgs()
    video_name = args.video_name
    message = args.message
    template_dir = args.template_dir
    confidence = args.confidence

    return video_name, message, template_dir, confidence


def MainImgRecog():
    video_name, message, template_dir, confidence = GetDataFromArgs()

    print(f"Searching for: {video_name}")
    print(f"Message: {message}")

    searchIcons = [f"{template_dir}/SearchInput1-2160.png", f"{template_dir}/SearchInput1-1080.png"]
    searchInputs = [f"{template_dir}/SearchInput2-2160.png", f"{template_dir}/SearchInput2-1080.png"]
    searchInputMoves = [f"{template_dir}/SearchInputMoveDown-2160.png", f"{template_dir}/SearchInputMoveDown-1080.png"]
    searchResults = [f"{template_dir}/SearchResult-2160.png", f"{template_dir}/SearchResult-1080.png"]
    cacheVideos = [f"{template_dir}/CacheVideo-2160.png", f"{template_dir}/CacheVideo-1080.png"]
    downloadVideos = [f"{template_dir}/DownloadVideo-2160.png", f"{template_dir}/DownloadVideo-1080.png"]
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
        state = CheckAndProcessCacheVideo(cacheVideos, confidence, state)
        state = CheckAndProcessDownloadVideo(downloadVideos, confidence, state)

        if state == state_unknown or state == state_download_video:
            SaveScreenshot(debug_png)
            print(f"The last state is {state}, break the loop.")
            break
        
        if counter == 10:
            print("Timeout! Current state: ", state)
            SaveScreenshot(debug_png)
            print(f"Debug screenshot saved to {debug_png}")
            break

    print("\nDone!")


def GetIndexByText(results, text):
    for i, line in enumerate(results):
        if text in line:
            return i
    return -1


def GetBoxByText(results, text):
    index = GetIndexByText(results[0]['rec_texts'], text)
    if index == -1:
        return None

    box = results[0]['rec_boxes'][index]
    return box


def ClickByBox(box, type="center"):
    if box is None:
        return False

    width = box[2] - box[0]
    height = box[3] - box[1]

    if type == "center":
        x = box[0] + width / 2
        y = box[1] + height / 2
    elif type == "top_left":
        x = box[0] + 5
        y = box[1] + 5
    else:
        x = box[0] + width / 2
        y = box[1] + height / 2

    pyautogui.click(x, y)
    time.sleep(0.5)
    return True


def DebugSaveResults(img_np, boxes, texts, index):
    for box, text in zip(boxes, texts):
        #box = np.array(line[0]).astype(np.int32).reshape((-1, 1, 2))

        # 1. 将 list 转换为 numpy 数组
        # 2. 强制转换为 int32 (CV_32S)
        # 3. reshape 为 (-1, 1, 2) 
        box_np = np.array(box).astype(np.int32).reshape((-1, 1, 2))

        cv2.polylines(img_np, [box_np], True, (0, 255, 0), 2)
        cv2.putText(img_np, text, (box_np[0][0][0], box_np[0][0][1]-10), 
                    cv2.FONT_HERSHEY_SIMPLEX, 0.5, (0, 255, 0), 1)

    debug_png = f"./scripts/BiliBiliDownloader/ocr_debug_m{index}.png"
    cv2.imwrite(debug_png, img_np)
    print(f"已保存调试图 {debug_png}，请检查绿框是否准确对准文字")


def MainOCR():
    global g_monitors

    video_name, message, template_dir, confidence = GetDataFromArgs()

    print(f"Searching for: {video_name}")
    print(f"Message: {message}")

    monitors = g_monitors

    index = 0
    for m in monitors:
        print(f"Monitor {index}: {m.width}x{m.height} at ({m.x}, {m.y})") 
        index += 1

        # 初始化 OCR 引擎（第一次运行会自动下载模型）
        ocr = PaddleOCR(use_angle_cls=True,
                        lang='ch',
                        det_limit_side_len=1350,
                        use_doc_unwarping=False,
                        det_db_unclip_ratio=1.2)

        # 1. 截图并转换为 NumPy 数组供 OCR 使用
        screenshot = ScreenshotMonitor(m)

        debug_png = f"./scripts/BiliBiliDownloader/bilibili_ocr_test_m{index}.png"
        screenshot.save(debug_png) 
        img_np = np.array(screenshot)

        # 确保传给 OCR 的是 BGR 格式
        img_for_ocr = cv2.cvtColor(img_np, cv2.COLOR_RGBA2BGR)

        # 确保传给 OCR 的是 BGR 格式
        #img_for_ocr = cv2.cvtColor(img_np, cv2.COLOR_RGBA2GRAY)

        # 目标尺寸 (Width, Height)
        # target_size = (1920, 1080)

        # 执行缩放
        # 建议使用 INTER_AREA，它在缩小图片时能更好地保留文字边缘像素
        # img_resized = cv2.resize(img_for_ocr, target_size, interpolation=cv2.INTER_AREA)

        # 2. 执行识别
        results = ocr.predict(img_for_ocr)
        # results = ocr.ocr(img_np)
        # results = ocr.predict(debug_png)

        DebugSaveResults(img_for_ocr, results[0]['rec_polys'], results[0]['rec_texts'], index)

        # Find "搜索你感兴趣的视频"
        text = "搜索你感兴趣的视频"
        box = GetBoxByText(results, text)
        ClickByBox(box)

        print(f"{text} OCR Results: {box}")

    return


if __name__ == "__main__":
    # MainImgRecog()
    MainOCR()
