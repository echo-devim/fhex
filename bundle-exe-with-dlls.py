import os
import time
from shutil import copyfile

# Determine the destination directory
dest_dir="fhex-{}-{}".format(time.strftime("%Y%m%d%H%M%S"), os.environ['GITHUB_SHA'])
# Set it as an environment variable (for asset uploading)
print("::set-env name=BUNDLE_DIR::{}".format(dest_dir))

os.mkdir(dest_dir)

# Copy the executable there
for file in os.listdir('release'):
	if file[-4:] != '.exe':
		continue
	copyfile("release/{}".format(file), "{}/{}".format(dest_dir, file))

# Now, find out the dependencies the executable need and include them (unless they're from Windows)
for file in os.listdir(dest_dir):
	deps = os.popen("dumpbin /DEPENDENTS {}\\{}".format(dest_dir, file))
	print("Found dependencies")
	print(deps)
	deps = deps.readlines()
	print("deps list")
	print(deps)
	for dep in deps:
		dep = dep.strip() # Remove trailing line break and leading spaces
		print("Dep is {}".format(dep))
		if dep[-4:] != ".dll":
			continue
		# Find the DLL
		dll_path = os.popen("where {}".format(dep))
		dll_path = dll_path.readlines()
		dll_path = dll_path[0].strip()
		print("dll path is {}", dll_path)
		# If it's in system 32, it's safe to assume it's provided by Windows
		if dll_path.lower().startswith("c:\\windows\\system32"):
			continue
		copyfile(dll_path, "{}/{}".format(dest_dir, dep))
		print("Copied {}".format(dep))
		
print("All done! The .exe file should work!")		
