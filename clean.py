import os

clean_extensions = [".asm", ".exe", ".ilk", ".obj", "pdb", ".lib", ".ppm"]

num_files_removed = 0

for filename in os.listdir("Examples"):
    for ext in clean_extensions:
        if (filename.endswith(ext)):
            os.remove("Examples/" + filename)
            num_files_removed += 1
            break

print("Removed {} files".format(num_files_removed))
