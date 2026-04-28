import sys
from PIL import Image

def compare_images(img1_path, img2_path):
    img1 = Image.open(img1_path)
    img2 = Image.open(img2_path)

    if img1.size != img2.size:
        print(f"FAIL: Size mismatch - {img1.size} vs {img2.size}")
        return False

    pixels1 = list(img1.getdata())
    pixels2 = list(img2.getdata())

    differences = 0
    tolerance = 30 # pixel channel value's tolerance
    for i, (p1, p2) in enumerate(zip(pixels1, pixels2)):
        # if p1 != p2:
        
        if abs(p1[0] - p2[0]) >= tolerance or\
            abs(p1[1] - p2[1]) >= tolerance or\
            abs(p1[2] - p2[2]) >= tolerance:
            differences += 1
            if differences <= 10:  # Show first 10 differences
                x = i % img1.size[0]
                y = i // img1.size[0]
                print(f"  Diff at ({x}, {y}): {p1} vs {p2}")

    total_pixels = img1.size[0] * img1.size[1]

    # If there is less than 100 different pixels, we take it as passed.
    pixelsPassRate = 0.9
    tolerancePixels = int(total_pixels * (1.0 - pixelsPassRate)) 
    if differences <= tolerancePixels:
        print(f"Total: {total_pixels} PASS: {total_pixels-differences} pixels match!, {100*(total_pixels - differences)/total_pixels:.2f}%")
        return True
    else:
        print(f"FAIL: {differences}/{total_pixels} pixels differ ({100*differences/total_pixels:.2f}%)")
        return False

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <example_name>")
        print(f"Example: python {sys.argv[0]} 01.HelloWorld")
        # sys.exit(1)
        example_name = "02.Quake3Map"
    else:
        example_name = sys.argv[1]

    screenshot = f"examples/{example_name}/screenshot.bmp"
    golden = f"examples/{example_name}/Frame1-screenshot-golden.bmp"

    compare_images(screenshot, golden)
