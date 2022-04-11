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
#include "../random.h"

#include "speaker.h"

#include "../stage/week7.h"

#define random RandomRange(0, 16)
//picosp character structure
enum
{
	picosp_ArcMain_Idle,
	picosp_ArcMain_Left0,
	picosp_ArcMain_Left1,
    picosp_ArcMain_Right0,
    picosp_ArcMain_Right1,
	
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
	{picosp_ArcMain_Idle, {  0,   0, 113, 121}, { 37,  72}}, //0 bop left 1
	{picosp_ArcMain_Idle, {113,   0, 126, 103}, { 37,  72}}, //1 bop left 2
	{picosp_ArcMain_Idle, {  0, 121, 139, 101}, { 37,  72}}, //2 bop left 3
	{picosp_ArcMain_Idle, {139, 103, 115, 112}, { 37,  72}}, //3 bop left 4
	
	{picosp_ArcMain_Left0, {  0,   0, 172, 101}, { 37,  72}}, //4 bop right 1
	{picosp_ArcMain_Left0, {  0, 101, 172, 101}, { 37,  72}}, //5 bop right 2
    {picosp_ArcMain_Left1, { 17,   0, 117, 116}, { 37,  72}}, //5 bop right 2

	{picosp_ArcMain_Right0, {  0,   0, 171,  89}, { 37,  72}}, //6 bop right 4
	{picosp_ArcMain_Right0, {  0,  89, 172,  89}, { 37,  72}}, //7 bop right 5
    {picosp_ArcMain_Right1, { 13,   0, 113, 117}, { 37,  72}}, //6 bop right 4
};

static const Animation char_picosp_anim[CharAnim_Max] = {
	{2, (const u8[]){0, 1, 2, 3, ASCR_CHGANI, 0}},                           //CharAnim_Idle
	{2, (const u8[]){4, 5, 6, ASCR_BACK, 0}},                                   //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_LeftAlt
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_DownAlt
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_UpAlt
	{2, (const u8[]){7, 8, 9, ASCR_BACK, 0}},                                   //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, 0}},                                       //CharAnim_RightAlt
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
                if (random == 8)
				    character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_RightAlt : CharAnim_LeftAlt);
				else
                    character->set_anim(character, ((*this->pico_p) & 0x8000) ? CharAnim_Right : CharAnim_Left);

                Speaker_Bump(&this->speaker);
				this->pico_p++;
			}
		}
	}
	else
	{
		if (stage.flag & STAGE_FLAG_JUST_STEP)
		{
            //Bump speakers
			if (stage.note_scroll >= character->sing_end && (stage.song_step % stage.gf_speed) == 0)
				Speaker_Bump(&this->speaker);
			
		}
	}
    
    //Get random animation numbers
    random;

	//Get parallax
	fixed_t parallax;

	if (stage.stage_id >= StageId_1_1 && stage.stage_id <= StageId_1_4)
		parallax = FIXED_DEC(7,10);
	else
		parallax = FIXED_UNIT;
	
    //Perform idle dance
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0)
		Character_PerformIdle(character);

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
