#!/usr/bin/env python3
"""
Bilibili Video Downloader - Search, cache and download videos

Usage:
    python bilibili_downloader.py "search query"
"""

import sys
import argparse
from playwright.sync_api import sync_playwright


def main():
    parser = argparse.ArgumentParser(description="Bilibili Video Downloader")
    parser.add_argument("search_query", type=str, nargs="?", help="Video to search for")
    parser.add_argument("--headless", action="store_true", default=False)
    parser.add_argument("--delay", type=int, default=2000, help="Delay in ms")
    
    args = parser.parse_args()
    
    if not args.search_query:
        parser.print_help()
        print("\nExample: python bilibili_downloader.py \"Python教程\"")
        sys.exit(1)
    
    search_query = args.search_query
    delay = args.delay / 1000
    
    print(f"Searching for: {search_query}")
    
    with sync_playwright() as p:
        browser = p.chromium.launch(headless=args.headless)
        context = browser.new_context(viewport={"width": 1280, "height": 720}, locale="zh-CN")
        page = context.new_page()
        
        try:
            page.goto("https://www.bilibili.com")
            page.wait_for_load_state("networkidle")
            page.wait_for_timeout(delay)
            
            search_selectors = [
                "input[placeholder*='搜索']",
                "input[placeholder*='搜索视频']",
                ".nav-search-input",
                "#search-input"
            ]
            
            search_input = None
            for selector in search_selectors:
                try:
                    search_input = page.locator(selector).first
                    if search_input.is_visible():
                        break
                except:
                    continue
            
            if not search_input or not search_input.is_visible():
                page.keyboard.press("/")
                page.wait_for_timeout(500)
            
            search_input.fill(search_query)
            page.wait_for_timeout(delay / 2)
            search_input.press("Enter")
            page.wait_for_load_state("networkidle")
            page.wait_for_timeout(delay)
            
            result_selectors = [
                ".video-item a",
                ".video-card a", 
                ".search-result .video-item a",
                "a[href*='/video/']"
            ]
            
            first_result = None
            for selector in result_selectors:
                try:
                    first_result = page.locator(selector).first
                    if first_result.is_visible():
                        break
                except:
                    continue
            
            if first_result:
                try:
                    title = first_result.get_attribute("title") or "Unknown"
                    print(f"Opening: {title}")
                except:
                    pass
                
                first_result.click()
                page.wait_for_load_state("networkidle")
                page.wait_for_timeout(delay)
                
                cache_button_selectors = [
                    "button:has-text('缓存')",
                    ".cache-btn",
                    "button[class*='cache']",
                    "button[class*='downloading']"
                ]
                
                cache_button = None
                for selector in cache_button_selectors:
                    try:
                        cache_button = page.locator(selector).first
                        if cache_button.is_visible():
                            break
                    except:
                        continue
                
                if cache_button:
                    cache_button.click()
                    page.wait_for_timeout(delay)
                    
                    download_button_selectors = [
                        "button:has-text('下载')",
                        ".download-btn",
                        "button[class*='download']",
                        "button:has-text('开始下载')"
                    ]
                    
                    download_button = None
                    for selector in download_button_selectors:
                        try:
                            download_button = page.locator(selector).first
                            if download_button.is_visible():
                                break
                        except:
                            continue
                    
                    if download_button:
                        download_button.click()
                        page.wait_for_timeout(delay)
                        print("Download started!")
                    else:
                        print("Download may have started automatically after caching")
                else:
                    print("Cache button not found. Screenshot saved.")
                    page.screenshot(path="bilibili_debug.png")
            else:
                print("No search results found")
                
        except Exception as e:
            print(f"Error: {e}")
            try:
                page.screenshot(path="bilibili_error.png")
            except:
                pass
            raise
        finally:
            page.wait_for_timeout(5000)
            browser.close()
            
    print("\nDone!")


if __name__ == "__main__":
    main()
