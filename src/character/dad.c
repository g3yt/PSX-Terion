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
	Dad_ArcMain_Idle2,
	Dad_ArcMain_Left0,
	Dad_ArcMain_Left1,
	Dad_ArcMain_Down0,
	Dad_ArcMain_Down1,
	Dad_ArcMain_Up0,
	Dad_ArcMain_Up1,
	Dad_ArcMain_Up2,
	Dad_ArcMain_Right0,
	Dad_ArcMain_Right1,


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
  {Dad_ArcMain_Idle0, {  0,  0, 82,118}, {153,156}}, //0 idle 1
  {Dad_ArcMain_Idle0, { 82,  0, 91,117}, {158,155}}, //1 idle 2
  {Dad_ArcMain_Idle0, {  0,118, 89,117}, {155,155}}, //2 idle 3
  {Dad_ArcMain_Idle0, { 89,118, 87,117}, {154,155}}, //3 idle 4
  {Dad_ArcMain_Idle1, {  0,  0, 84,117}, {154,156}}, //4 idle 5
  {Dad_ArcMain_Idle1, { 84,  0, 82,118}, {153,157}}, //5 idle 6
  {Dad_ArcMain_Idle1, {166,  0, 81,118}, {153,157}}, //6 idle 7
  {Dad_ArcMain_Idle1, {  0,118, 81,118}, {153,157}}, //7 idle 8
  {Dad_ArcMain_Idle1, { 81,118, 80,118}, {153,157}}, //8 idle 9
  {Dad_ArcMain_Idle1, {161,118, 81,118}, {153,157}}, //9 idle 10
  {Dad_ArcMain_Idle2, {  0,  0, 82,118}, {153,157}}, //10 idle 11
  {Dad_ArcMain_Idle2, { 82,  0, 82,118}, {153,157}}, //11 idle 12

  {Dad_ArcMain_Left0, {  0,  0, 78,113}, {180,152}}, //12 left 1
  {Dad_ArcMain_Left0, { 78,  0, 80,110}, {191,151}}, //13 left 2
  {Dad_ArcMain_Left0, {158,  0, 76,112}, {184,152}}, //14 left 3
  {Dad_ArcMain_Left0, {  0,113, 77,112}, {183,151}}, //15 left 4
  {Dad_ArcMain_Left0, { 77,113, 78,113}, {182,152}}, //16 left 5
  {Dad_ArcMain_Left0, {155,113, 77,114}, {181,153}}, //17 left 6
  {Dad_ArcMain_Left1, {  0,  0, 77,115}, {182,154}}, //18 left 7
  {Dad_ArcMain_Left1, { 77,  0, 77,115}, {182,154}}, //19 left 8
  {Dad_ArcMain_Left1, {154,  0, 77,114}, {180,153}}, //20 left 9
  {Dad_ArcMain_Left1, {  0,115, 77,113}, {179,152}}, //21 left 10
  {Dad_ArcMain_Left1, { 77,115, 77,113}, {179,152}}, //22 left 11

  {Dad_ArcMain_Down0, {  0,  0, 86,113}, {139,151}}, //23 down 1
  {Dad_ArcMain_Down0, { 86,  0, 82,115}, {141,155}}, //24 down 2
  {Dad_ArcMain_Down0, {168,  0, 79,115}, {141,157}}, //25 down 3
  {Dad_ArcMain_Down0, {  0,116, 78,116}, {141,158}}, //26 down 4
  {Dad_ArcMain_Down0, { 78,116, 79,115}, {142,157}}, //27 down 5
  {Dad_ArcMain_Down0, {157,116, 79,115}, {141,157}}, //28 down 6
  {Dad_ArcMain_Down1, {  0,  0, 79,115}, {141,157}}, //29 down 7
  {Dad_ArcMain_Down1, { 79,  0, 79,115}, {141,157}}, //30 down 8
  {Dad_ArcMain_Down1, {158,  0, 79,115}, {141,157}}, //31 down 9

  {Dad_ArcMain_Up0, {  0,  0, 78,127}, {133,202}}, //32 up 1
  {Dad_ArcMain_Up0, { 78,  0, 79,118}, {129,190}}, //33 up 2
  {Dad_ArcMain_Up0, {157,  0, 78,116}, {128,185}}, //34 up 3
  {Dad_ArcMain_Up0, {  0,127, 78,116}, {129,185}}, //35 up 4
  {Dad_ArcMain_Up0, { 78,127, 78,117}, {129,186}}, //36 up 5
  {Dad_ArcMain_Up0, {156,127, 78,117}, {128,187}}, //37 up 6
  {Dad_ArcMain_Up1, {  0,  0, 87,109}, {133,173}}, //38 up 7
  {Dad_ArcMain_Up1, { 87,  0, 92,106}, {135,168}}, //39 up 8
  {Dad_ArcMain_Up1, {  0,109, 93,107}, {136,169}}, //40 up 9
  {Dad_ArcMain_Up1, { 93,109, 93,107}, {136,169}}, //41 up 10
  {Dad_ArcMain_Up2, {  0,  0, 92,107}, {135,169}}, //42 up 11

  {Dad_ArcMain_Right0, {  0,  0, 96,108}, {142,151}}, //43 right 1
  {Dad_ArcMain_Right0, { 96,  0, 85,112}, {140,154}}, //44 right 2
  {Dad_ArcMain_Right0, {  0,117, 81,117}, {138,157}}, //45 right 3
  {Dad_ArcMain_Right0, { 81,117, 72,118}, {138,158}}, //46 right 4
  {Dad_ArcMain_Right0, {153,117, 72,120}, {141,159}}, //47 right 5
  {Dad_ArcMain_Right1, {  0,  0, 71,120}, {141,159}}, //48 right 6
  {Dad_ArcMain_Right1, { 71,  0, 71,120}, {142,159}}, //49 right 7
  {Dad_ArcMain_Right1, {142,  0, 71,120}, {141,159}}, //50 right 8
  {Dad_ArcMain_Right1, {  0,120, 72,120}, {141,159}}, //51 right 9
  {Dad_ArcMain_Right1, { 72,120, 72,120}, {140,159}}, //52 right 10
  {Dad_ArcMain_Right1, {144,120, 72,120}, {140,159}}, //53 right 11
};

static const Animation char_dad_anim[CharAnim_Max] = {
	{1, (const u8[]){ 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, ASCR_BACK, 1}}, //CharAnim_Idle
	{2, (const u8[]){ 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22,  ASCR_BACK, 1}},         //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_LeftAlt
	{2, (const u8[]){ 23, 24, 25, 26, 27, 28, 29, 30, 31, ASCR_BACK, 1}},         //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},   //CharAnim_DownAlt
	{2, (const u8[]){ 32, 33, 34, 35, 36, 37, 38, 39, 40, 41,  42, ASCR_BACK, 1}},         //CharAnim_Up
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
	this->character.focus_y = FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(12,10);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\DAD.ARC;1");
	
	const char **pathp = (const char *[]){
  "idle0.tim",
  "idle1.tim",
  "idle2.tim",
  "left0.tim",
  "left1.tim",
  "down0.tim",
  "down1.tim",
  "up0.tim",
  "up1.tim",
  "up2.tim",
  "right0.tim",
  "right1.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
