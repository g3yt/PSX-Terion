# PSXFunkin
Friday Night Funkin' on the PSX LOL

## Code samples
```
// Note shake : 
// Add this code in stage.c, case StageState_Play:

if (stage.song_step >= "your step") // Replace "your step" with the step when you want noteshake to start
    noteshake = 1;
else
    noteshake = 0;

// Sprite angle changing :

// Draw sprite
fx = stage.camera.x;
fy = stage.camera.y;
	
int sprite_x = FIXED_DEC(80,1); // X position of the sprite
int sprite_y = FIXED_DEC(30,1); // Y position of the sprite

u8 sprite_angle = 0; // Angle of the sprite

RECT sprite_src = {0, 0, 256, 256};

Gfx_DrawTexRotate(&this->tex_sprite, sprite_x, sprite_y, &sprite_src, sprite_angle, stage.camera.bzoom, fx, fy);
```
## Compilation
Refer to [COMPILE.md](/COMPILE.md) here

## Disclaimer
This project is not endorsed by the original Friday Night Funkin' devs, this is just an unofficial fan project because I was bored.
