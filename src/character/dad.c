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
	Dad_ArcMain_Dad0,
	Dad_ArcMain_Dad1,
	Dad_ArcMain_Dad2,
	Dad_ArcMain_Dad3,
	Dad_ArcMain_Dad4,
	Dad_ArcMain_Dad5,
	Dad_ArcMain_Dad6,
	Dad_ArcMain_Dad7,
	Dad_ArcMain_Dad8,
	Dad_ArcMain_Dad9,

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
  {Dad_ArcMain_Dad0, {  2,  0, 81,117}, { 93, 96}}, //0 idle 1
  {Dad_ArcMain_Dad0, { 85,  0, 90,116}, { 98, 95}}, //1 idle 2
  {Dad_ArcMain_Dad0, {  0,119, 88,116}, { 95, 95}}, //2 idle 3
  {Dad_ArcMain_Dad0, { 90,119, 86,116}, { 95, 96}}, //3 idle 4
  {Dad_ArcMain_Dad1, {  0,  0, 82,116}, { 94, 95}}, //4 idle 5
  {Dad_ArcMain_Dad1, { 84,  0, 82,117}, { 94, 96}}, //5 idle 6
  {Dad_ArcMain_Dad1, {168,  0, 80,117}, { 93, 96}}, //6 idle 7
  {Dad_ArcMain_Dad1, {  0,119, 80,117}, { 93, 96}}, //7 idle 8
  {Dad_ArcMain_Dad1, { 82,119, 80,117}, { 93, 96}}, //8 idle 9
  {Dad_ArcMain_Dad1, {164,119, 81,117}, { 93, 96}}, //9 idle 10
  {Dad_ArcMain_Dad2, {  0,  0, 81,117}, { 93, 96}}, //10 idle 11
  {Dad_ArcMain_Dad2, { 83,  0, 82,117}, { 93, 96}}, //11 idle 12

  {Dad_ArcMain_Dad2, {167,  0, 77,111}, {121, 91}}, //12 left 1
  {Dad_ArcMain_Dad2, {  0,119, 79,110}, {132, 91}}, //13 left 2
  {Dad_ArcMain_Dad2, { 81,119, 75,111}, {124, 91}}, //14 left 3
  {Dad_ArcMain_Dad2, {158,119, 76,112}, {123, 91}}, //15 left 4
  {Dad_ArcMain_Dad3, {  0,  0, 77,112}, {122, 91}}, //16 left 5
  {Dad_ArcMain_Dad3, { 79,  0, 77,113}, {122, 92}}, //17 left 6
  {Dad_ArcMain_Dad3, {158,  0, 76,114}, {121, 93}}, //18 left 7
  {Dad_ArcMain_Dad3, {  0,116, 76,114}, {121, 93}}, //19 left 8
  {Dad_ArcMain_Dad3, { 78,116, 77,113}, {120, 92}}, //20 left 9
  {Dad_ArcMain_Dad3, {157,116, 76,113}, {120, 92}}, //21 left 10
  {Dad_ArcMain_Dad4, {  0,  0, 76,113}, {120, 92}}, //22 left 11

  {Dad_ArcMain_Dad4, { 78,  0, 85,112}, { 80, 92}}, //23 down 1
  {Dad_ArcMain_Dad4, {165,  0, 81,113}, { 81, 95}}, //24 down 2
  {Dad_ArcMain_Dad4, {  0,115, 78,114}, { 81, 97}}, //25 down 3
  {Dad_ArcMain_Dad4, { 80,115, 77,115}, { 81, 98}}, //26 down 4
  {Dad_ArcMain_Dad4, {159,115, 78,114}, { 81, 97}}, //27 down 5
  {Dad_ArcMain_Dad5, {  0,  0, 78,114}, { 81, 97}}, //28 down 6
  {Dad_ArcMain_Dad5, { 80,  0, 78,114}, { 81, 97}}, //29 down 7
  {Dad_ArcMain_Dad5, {160,  0, 78,114}, { 81, 97}}, //30 down 8
  {Dad_ArcMain_Dad5, {  0,116, 78,114}, { 81, 97}}, //31 down 9

  {Dad_ArcMain_Dad5, { 80,116, 76,126}, { 74,142}}, //32 up 1
  {Dad_ArcMain_Dad5, {158,116, 78,117}, { 71,130}}, //33 up 2
  {Dad_ArcMain_Dad6, {  0,  0, 77,115}, { 69,126}}, //34 up 3
  {Dad_ArcMain_Dad6, { 79,  0, 78,115}, { 69,125}}, //35 up 4
  {Dad_ArcMain_Dad6, {159,  0, 77,116}, { 68,126}}, //36 up 5
  {Dad_ArcMain_Dad6, {  0,118, 76,116}, { 68,126}}, //37 up 6
  {Dad_ArcMain_Dad6, { 78,118, 86,108}, { 73,112}}, //38 up 7
  {Dad_ArcMain_Dad7, {  0,  0, 91,106}, { 75,108}}, //39 up 8
  {Dad_ArcMain_Dad7, { 93,  0, 92,106}, { 76,108}}, //40 up 9
  {Dad_ArcMain_Dad7, {  0,108, 91,107}, { 75,109}}, //41 up 10
  {Dad_ArcMain_Dad7, { 93,108, 90,107}, { 75,109}}, //42 up 11

  {Dad_ArcMain_Dad8, {  0,  0, 95,108}, { 82, 91}}, //43 right 1
  {Dad_ArcMain_Dad8, { 97,  0, 84,112}, { 80, 95}}, //44 right 2
  {Dad_ArcMain_Dad8, {  0,114, 80,115}, { 77, 97}}, //45 right 3
  {Dad_ArcMain_Dad8, { 82,114, 71,117}, { 78, 98}}, //46 right 4
  {Dad_ArcMain_Dad8, {155,114, 70,119}, { 80, 99}}, //47 right 5
  {Dad_ArcMain_Dad9, {  0,  0, 71,119}, { 82, 99}}, //48 right 6
  {Dad_ArcMain_Dad9, { 73,  0, 70,119}, { 81, 99}}, //49 right 7
  {Dad_ArcMain_Dad9, {145,  0, 70,119}, { 81, 99}}, //50 right 8
  {Dad_ArcMain_Dad9, {  0,121, 70,119}, { 80, 99}}, //51 right 9
  {Dad_ArcMain_Dad9, { 72,121, 70,118}, { 80, 99}}, //52 right 10
  {Dad_ArcMain_Dad9, {144,121, 71,118}, { 80, 99}}, //53 right 11
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
	"dad0.tim",
	"dad1.tim",
	"dad2.tim",
	"dad3.tim",
	"dad4.tim",
	"dad5.tim",
	"dad6.tim",
	"dad7.tim",
	"dad8.tim",
	"dad9.tim",
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	return (Character*)this;
}
