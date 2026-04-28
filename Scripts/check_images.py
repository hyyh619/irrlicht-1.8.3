import sys
from PIL import Image

if __name__ == "__main__":
    if len(sys.argv) != 2:
        print(f"Usage: python {sys.argv[0]} <example_name>")
        print(f"Example: python {sys.argv[0]} 01.HelloWorld")
        sys.exit(1)

    example_name = sys.argv[1]
    screenshot = Image.open(f"examples/{example_name}/screenshot.bmp")
    golden = Image.open(f"examples/{example_name}/Frame1-screenshot-golden.bmp")

    print(f"Screenshot: size={screenshot.size}, mode={screenshot.mode}, format={screenshot.format}")
    print(f"Golden: size={golden.size}, mode={golden.mode}, format={golden.format}")