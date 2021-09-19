.PHONY : all

#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
UNAME_S := $(shell uname -s)
ifeq ($(OS),Windows_NT)
	BLENDER = blender
else
	ifeq ($(UNAME_S),Darwin)
		BLENDER = /Applications/Blender/blender.app/Contents/MacOS/blender -y
	else ifeq ($(UNAME_S),Linux)
		BLENDER = ../../blender-2.93.4-linux-x64/blender -y
	else
		BLENDER = blender
	endif
endif

EXPORT_MESHES=export-meshes.py
EXPORT_SCENE=export-scene.py

DIST=../dist

all : \
	$(DIST)/NTFC.pnct \
	$(DIST)/NTFC.scene \
	$(info [${BLENDER}])


$(DIST)/NTFC.scene : NTFC.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- '$<':Main '$@'

$(DIST)/NTFC.pnct : NTFC.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- '$<':Main '$@'
