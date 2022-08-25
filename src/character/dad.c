/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "dad.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

//Dad character structure
enum
{
	Dad_ArcMain_Idle0,
	Dad_ArcMain_Idle1,
	Dad_ArcMain_Left,
	Dad_ArcMain_Down,
	Dad_ArcMain_Down1,
	Dad_ArcMain_Up,
	Dad_ArcMain_Up1,
	Dad_ArcMain_Up2,
	Dad_ArcMain_Right,
	
	Dad_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[Dad_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
} Char_Dad;

//Dad character definitions
static const CharFrame char_dad_frame[] = {
	{Dad_ArcMain_Idle0, {  0,   0, 81, 117}, { 31, 99+4}}, //0 idle 1
	{Dad_ArcMain_Idle0, {81,   0, 90, 116}, { 36, 98+4}}, //1 idle 2
	{Dad_ArcMain_Idle0, {  0,   140, 88, 116}, { 33, 98+4}}, //2 idle 3
	{Dad_ArcMain_Idle0, {88,   140, 86, 116}, { 32, 99+4}}, //3 idle 4
	{Dad_ArcMain_Idle0, {171,   0, 82, 116}, { 32, 98+4}}, //2 idle 3
	{Dad_ArcMain_Idle0, {174,   139, 82, 117}, { 32, 99+4}}, //3 idle 4
	{Dad_ArcMain_Idle1, {  0,   0, 80, 117}, { 31, 99+4}}, //2 idle 3
	{Dad_ArcMain_Idle1, {80,   4, 80, 117}, { 31, 99+4}}, //3 idle 4
	{Dad_ArcMain_Idle1, {  166,   2, 80, 117}, { 31, 99+4}}, //2 idle 3
	{Dad_ArcMain_Idle1, {0,   139, 81, 117}, { 31, 99+4}}, //3 idle 4
	{Dad_ArcMain_Idle1, {  83,   139, 81, 117}, { 31, 99+4}}, //2 idle 3
	{Dad_ArcMain_Idle1, {164,   139, 82, 117}, { 31, 99+4}}, //3 idle 4

	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, { 94,   0,  95, 195}, { 40, 185+4}}, //5 left 2
	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, { 94,   0,  95, 195}, { 40, 185+4}}, //5 left 2
	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, { 94,   0,  95, 195}, { 40, 185+4}}, //5 left 2
	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, { 94,   0,  95, 195}, { 40, 185+4}}, //5 left 2
	{Dad_ArcMain_Left, {  0,   0,  93, 195}, { 40, 185+4}}, //4 left 1
	{Dad_ArcMain_Left, { 94,   0,  95, 195}, { 40, 185+4}}, //5 left 2

	
	{Dad_ArcMain_Down, {  0,   0, 85, 112}, { 43, 99+4}}, //6 down 1
	{Dad_ArcMain_Down, {76,    0, 78, 117}, { 43, 99+4}}, //7 down 2
	{Dad_ArcMain_Down, {154,   0, 77, 115}, { 43, 99+4}},
	{Dad_ArcMain_Down, {0,   126, 78, 115}, { 43, 99+4}},
	{Dad_ArcMain_Down, {79,   117, 76, 116}, { 43, 99+4}},
	{Dad_ArcMain_Down, {155,   142, 78, 114}, { 43, 99+4}},
	{Dad_ArcMain_Down1, {0,   0, 78, 114}, { 43, 99+4}},
	{Dad_ArcMain_Down1, {78,   0, 78, 114}, { 43, 99+4}},
	{Dad_ArcMain_Down1, {156,   0, 78, 114}, { 43, 99+4}},

	{Dad_ArcMain_Up, {  0,   0, 76, 126}, { 76, 126+4}}, //8 up 1
	{Dad_ArcMain_Up, {76,   0, 78, 117}, { 78, 117+4}}, //9 up 2
	{Dad_ArcMain_Up, {154,  0, 77, 115}, { 77, 115+4}}, //8 up 1
	{Dad_ArcMain_Up, {155,  115, 76, 116}, { 78, 115+4}}, //9 up 2
	{Dad_ArcMain_Up, { 78, 117, 77, 116}, { 77, 116+4}}, //8 up 1
	{Dad_ArcMain_Up, {  155,  115, 76, 166}, { 75, 116+4}}, //8 up 1
	{Dad_ArcMain_Up1, {  0,   0, 86, 108}, { 40, 194+4}}, //9 up 2
	{Dad_ArcMain_Up1, {86,   0, 91, 106}, { 40, 194+4}}, //9 up 2
	{Dad_ArcMain_Up1, {  0,  108, 92, 106}, { 40, 196+4}}, //8 up 1
	{Dad_ArcMain_Up1, {92,   108, 91, 107}, { 40, 194+4}}, //9 up 2
	{Dad_ArcMain_Up2, {  0,   0, 102, 205}, { 40, 196+4}}, //8 up 1

	{Dad_ArcMain_Right, {  0,   0, 117, 199}, { 43, 189+4}}, //10 right 1
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {  0,   0, 117, 199}, { 43, 189+4}}, //10 right 1
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {  0,   0, 117, 199}, { 43, 189+4}}, //10 right 1
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {  0,   0, 117, 199}, { 43, 189+4}}, //10 right 1
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {  0,   0, 117, 199}, { 43, 189+4}}, //10 right 1
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
	{Dad_ArcMain_Right, {118,   0, 114, 199}, { 42, 189+4}}, //11 right 2
};

static const Animation char_dad_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 23, 24, 25, 26, 27, 28, 29, 30, 31, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 32, 33, 34, 35, 36,   37, 38, 39, 40, 41,  42, ASCR_BACK, 1}},         //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_UpAlt
	{2, (const u8[]){43, 44, 45, 46, 47,  48, 49, 50, 51, 52, 53, ASCR_BACK, 1}},         //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_RightAlt
};

//Dad character functions
void Char_Dad_SetFrame(void *user, u8 frame)
{
	Char_Dad *this = (Char_Dad*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_dad_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_Dad_Tick(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_Dad_SetFrame);
	Character_Draw(character, &this->tex, &char_dad_frame[this->frame]);
}

void Char_Dad_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_Dad_Free(Character *character)
{
	Char_Dad *this = (Char_Dad*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_Dad_New(fixed_t x, fixed_t y)
{
	//Allocate dad object
	Char_Dad *this = Mem_Alloc(sizeof(Char_Dad));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_Dad_New] Failed to allocate dad object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_Dad_Tick;
	this->character.set_anim = Char_Dad_SetAnim;
	this->character.free = Char_Dad_Free;
	
	Animatable_Init(&this->character.animatable, char_dad_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 1;

	//health bar color
	this->character.health_bar = 0xFFAD63D6;
	
	this->character.focus_x = FIXED_DEC(65,1);
	this->character.focus_y = FIXED_DEC(-115,1);
	this->character.focus_zoom = FIXED_DEC(12,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\DAD.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle0.tim", //Dad_ArcMain_Idle0
		"idle1.tim", //Dad_ArcMain_Idle1
		"left.tim",  //Dad_ArcMain_Left
		"down.tim",  //Dad_ArcMain_Down
		"down1.tim",
		"up0.tim",    //Dad_ArcMain_Up
		"up1.tim",
		"up2.tim",
		"right.tim", //Dad_ArcMain_Right
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
