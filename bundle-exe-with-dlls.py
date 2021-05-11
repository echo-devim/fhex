import os
import time
from shutil import copyfile

# Determine the destination directory
dest_dir="fhex-{}-{}".format(time.strftime("%Y%m%d%H%M%S"), os.environ['GITHUB_SHA'])
# Set it as an environment variable (for the next steps)
with open(os.environ['GITHUB_ENV'], 'a') as f:
	f.writelines([
		"BUNDLE_DIR=%s" % dest_dir,
	])

os.mkdir(dest_dir)

# Copy the executable there
for file in os.listdir('release'):
	if file[-4:] != '.exe':
		continue
	copyfile("release/{}".format(file), "{}/{}".format(dest_dir, file))

# Now, find out the dependencies the executable need and include them (unless they're from Windows)
for file in os.listdir(dest_dir):
	deps = os.popen("dumpbin /DEPENDENTS {}\\{}".format(dest_dir, file)).readlines()
	print("Dumpbin reported the following DLLs")
	print(deps)
	for dep in deps:
		dep = dep.strip() # Remove trailing line break and leading spaces
		if dep[-4:] != ".dll":
			continue
		# Find the DLL
		dll_path = 'lib/%s' % dep
		# If not found in our custom lib directory, ask the system
		if not os.path.isfile(dll_path):
			dll_path = os.popen("where {}".format(dep)).readlines()
			dll_path = dll_path[0].strip()
		# If it's in system 32, it's safe to assume it's provided by Windows
		if dll_path.lower().startswith("c:\\windows\\system32"):
			print("Skipping {}", dll_path)
			continue
		copyfile(dll_path, "{}/{}".format(dest_dir, dep))
		print("Copied {}".format(dep))
		
print("All done! Now, run windeploy to get all QT libraries and you're good to go!")
