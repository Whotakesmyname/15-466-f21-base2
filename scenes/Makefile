
#n.b. the '-y' sets autoexec scripts to 'on' so that driver expressions will work
ifeq ($(OS),Windows_NT)
	BLENDER := "C:\Program Files\Blender Foundation\Blender 2.93\blender.exe"
else
	UNAME_S = $(shell uname -s)
	ifeq ($(UNAME_S),Darwin)
		BLENDER := /Applications/Blender/blender.app/Contents/MacOS/blender -y
	else ifeq ($(UNAME_S),Linux)
		BLENDER := ../../blender-2.93.4-linux-x64/blender -y
	else
		BLENDER := blender
	endif
endif

EXPORT_MESHES=export-meshes.py
EXPORT_SCENE=export-scene.py
EXPORT_TEX=pack_textures.py

DIST=../dist

.PHONY : all

all : \
	$(DIST)/NTFC.pnct \
	$(DIST)/NTFC.scene \
	$(DIST)/NTFC.tex


$(DIST)/NTFC.scene : NTFC.blend $(EXPORT_SCENE)
	$(BLENDER) --background --python $(EXPORT_SCENE) -- $<:Main $@

$(DIST)/NTFC.pnct : NTFC.blend $(EXPORT_MESHES)
	$(BLENDER) --background --python $(EXPORT_MESHES) -- $<:Main $@

$(DIST)/NTFC.tex : earth_cloud.jpg earth_daytime.jpg earth_night.jpg $(EXPORT_MESHES)
	python $(EXPORT_TEX)
