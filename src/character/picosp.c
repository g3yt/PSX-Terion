/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "picosp.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../main.h"

#include "speaker.h"

#include "../stage/week7.h"

//picosp character structure
enum
{
	picosp_ArcMain_picosp0,
	picosp_ArcMain_picosp1,
	picosp_ArcMain_picosp2,
	
	picosp_Arc_Max,
};

typedef struct
{
	//Character base structure
	Character character;
	
	//Render data and state
	IO_Data arc_main;
	IO_Data arc_ptr[picosp_Arc_Max];
	
	Gfx_Tex tex;
	u8 frame, tex_id;
	
	//Speaker
	Speaker speaker;
	
	//Pico test
	u16 *pico_p;
} Char_picosp;

//picosp character definitions
static const CharFrame char_picosp_frame[] = {
	{picosp_ArcMain_picosp0, {  0,   0,  74, 103}, { 37,  72}}, //0 bop left 1
	{picosp_ArcMain_picosp0, { 75,   0,  74, 103}, { 38,  72}}, //1 bop left 2
	{picosp_ArcMain_picosp0, {150,   0,  73, 102}, { 37,  72}}, //2 bop left 3
	{picosp_ArcMain_picosp0, {  0, 104,  73, 103}, { 36,  73}}, //3 bop left 4
	
	{picosp_ArcMain_picosp1, {  0,   0,  81, 104}, { 40,  73}}, //6 bop right 1
	{picosp_ArcMain_picosp1, { 82,   0,  81, 104}, { 40,  73}}, //7 bop right 2

	{picosp_ArcMain_picosp1, {  0, 104,  79, 103}, { 38,  74}}, //9 bop right 4
	{picosp_ArcMain_picosp1, { 80, 105,  74, 104}, { 32,  74}}, //10 bop right 5
};

static const Animation char_picosp_anim[CharAnim_Max] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_CHGANI, 0}},                           //CharAnim_Idle
	{2, (const u8[]){17, 18, ASCR_BACK, 1}},                                 //CharAnim_Left
	{1, (const u8[]){ 0,  0,  1,  1,  2,  2,  3,  4,  4,  5, ASCR_BACK, 1}}, //CharAnim_LeftAlt
	{2, (const u8[]){19, 20, ASCR_BACK, 1}},                                 //CharAnim_Down
	{1, (const u8[]){12, 13, 14, 15, 16, ASCR_REPEAT}},                      //CharAnim_DownAlt
	{2, (const u8[]){21, 22, ASCR_BACK, 1}},                                 //CharAnim_Up
	{2, (const u8[]){25, 26, ASCR_BACK, 1}},                                 //CharAnim_UpAlt
	{2, (const u8[]){23, 24, ASCR_BACK, 1}},                                 //CharAnim_Right
	{1, (const u8[]){ 6,  6,  7,  7,  8,  8,  9, 10, 10, 11, ASCR_BACK, 1}}, //CharAnim_RightAlt
};

//picosp character functions
void Char_picosp_SetFrame(void *user, u8 frame)
{
	Char_picosp *this = (Char_picosp*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_picosp_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_picosp_Tick(Character *character)
{
	Char_picosp *this = (Char_picosp*)character;
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL && this->pico_p == NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	
	if (this->pico_p != NULL)
	{
		if (stage.note_scroll >= 0)
		{
			//Scroll through Pico chart
			u16 substep = stage.note_scroll >> FIXED_SHIFT;
			while (substep >= ((*this->pico_p) & 0x7FFF))
			{
				//Play animation and bump speakers
				character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_RightAlt : CharAnim_LeftAlt);
				Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
            //Perform idle dance
            if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
                Character_PerformIdle(character);
			//Perform dance
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
				//Bump speakers
				Speaker_Bump(&this->speaker);
			
		}
	}
	
	//Get parallax
	fixed_t parallax;
	if (stage.stage_id >= StageId_1_1 && stage.stage_id <= StageId_1_4)
		parallax = FIXED_DEC(7,10);
	else
		parallax = FIXED_UNIT;
	
	//Animate and draw
	Animatable_Animate(&character->animatable, (void*)this, Char_picosp_SetFrame);
	Character_DrawParallax(character, &this->tex, &char_picosp_frame[this->frame], parallax);
	
	//Tick speakers
	Speaker_Tick(&this->speaker, character->x, character->y, parallax);
}

void Char_picosp_SetAnim(Character *character, u8 anim)
{
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_picosp_Free(Character *character)
{
	Char_picosp *this = (Char_picosp*)character;
	
	//Free art
	Mem_Free(this->arc_main);
}

Character *Char_picosp_New(fixed_t x, fixed_t y)
{
	//Allocate picosp object
	Char_picosp *this = Mem_Alloc(sizeof(Char_picosp));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_picosp_New] Failed to allocate picosp object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_picosp_Tick;
	this->character.set_anim = Char_picosp_SetAnim;
	this->character.free = Char_picosp_Free;
	
	Animatable_Init(&this->character.animatable, char_picosp_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = 0;
	
	this->character.health_i = 5;
	
	this->character.focus_x = FIXED_DEC(2,1);
	this->character.focus_y = FIXED_DEC(-40,1);
	this->character.focus_zoom = FIXED_DEC(1,1);
	
	//Load art
	this->arc_main = IO_Read("\\CHAR\\PICOSP.ARC;1");
	
	const char **pathp = (const char *[]){
		"idle.tim", //picosp_ArcMain_picosp0
		"left0.tim", //picosp_ArcMain_picosp1
		"left1.tim", //picosp_ArcMain_picosp2
        "right0.tim", //picosp_ArcMain_picosp1
		"right1.tim", //picosp_ArcMain_picosp2
		NULL
	};
	IO_Data *arc_ptr = this->arc_ptr;
	for (; *pathp != NULL; pathp++)
		*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize speaker
	Speaker_Init(&this->speaker);
	
	//Initialize Pico test
	if (stage.stage_id == StageId_7_3 && stage.back != NULL)
		this->pico_p = ((Back_Week7*)stage.back)->pico_chart;
	else
		this->pico_p = NULL;
	
	return (Character*)this;
}
