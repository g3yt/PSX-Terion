/*
  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "bf.h"

#include "../mem.h"
#include "../archive.h"
#include "../stage.h"
#include "../random.h"
#include "../main.h"

int secret;

//Boyfriend skull fragments
static SkullFragment char_bf_skull[15] = {
	{ 1 * 8, -87 * 8, -13, -13},
	{ 9 * 8, -88 * 8,   5, -22},
	{18 * 8, -87 * 8,   9, -22},
	{26 * 8, -85 * 8,  13, -13},
	
	{-3 * 8, -82 * 8, -13, -11},
	{ 8 * 8, -85 * 8,  -9, -15},
	{20 * 8, -82 * 8,   9, -15},
	{30 * 8, -79 * 8,  13, -11},
	
	{-1 * 8, -74 * 8, -13, -5},
	{ 8 * 8, -77 * 8,  -9, -9},
	{19 * 8, -75 * 8,   9, -9},
	{26 * 8, -74 * 8,  13, -5},
	
	{ 5 * 8, -73 * 8, -5, -3},
	{14 * 8, -76 * 8,  9, -6},
	{26 * 8, -67 * 8, 15, -3},
};

//Boyfriend player types
enum
{
	BF_ArcMain_Idle0,
	BF_ArcMain_Idle0,
	BF_ArcMain_BF2,
	BF_ArcMain_BF3,
	BF_ArcMain_BF4,
	BF_ArcMain_BF5,
	BF_ArcMain_BF6,
	BF_ArcMain_Dead0, //BREAK

	BF_ArcMain_Max,
};

enum
{
	BF_ArcDead_Dead1, //Mic Drop
	BF_ArcDead_Dead2, //Twitch
	BF_ArcDead_Retry, //Retry prompt

	BF_ArcDead_Max,
};

#define BF_Arc_Max BF_ArcMain_Max

typedef struct
{
	//Character base structure
	Character character;

	//Render data and state
	IO_Data arc_main, arc_dead;
	CdlFILE file_dead_arc; //dead.arc file position
	IO_Data arc_ptr[BF_Arc_Max];
	
	Gfx_Tex tex, tex_retry;
	u8 frame, tex_id;
	
	u8 retry_bump;
	
	SkullFragment skull[COUNT_OF(char_bf_skull)];
	u8 skull_scale;
} Char_BF;

//Boyfriend player definitions
static const CharFrame char_bf_frame[] = {
	{BF_ArcMain_Idle0, {  0,  0, 86,114}, {131,159}}, //0 idle 1
	{BF_ArcMain_Idle0, { 86,  0, 86,112}, {131,157}}, //1 idle 2
	{BF_ArcMain_Idle0, {  0,114, 86,108}, {131,152}}, //2 idle 3
	{BF_ArcMain_Idle0, { 86,114, 87,105}, {131,149}}, //3 idle 4
	{BF_ArcMain_Idle1, {  0,  0, 87,104}, {131,148}}, //4 idle 5
	{BF_ArcMain_Idle1, { 87,  0, 86,105}, {131,149}}, //5 idle 6
	{BF_ArcMain_Idle1, {  0,105, 86,105}, {131,150}}, //6 idle 7
	{BF_ArcMain_Idle1, { 86,105, 86,108}, {131,153}}, //7 idle 8
	{BF_ArcMain_Idle2, {  0,  0, 86,112}, {131,157}}, //8 idle 9
	{BF_ArcMain_Idle2, { 86,  0, 86,114}, {131,159}}, //9 idle 10
	{BF_ArcMain_Idle2, {  0,114, 86,114}, {131,160}}, //10 idle 11

	{BF_ArcMain_Left0, {  0,  0, 91,120}, {135,165}}, //11 left 1
	{BF_ArcMain_Left0, { 91,  0, 89,119}, {134,164}}, //12 left 2
	{BF_ArcMain_Left0, {  0,120, 88,117}, {133,162}}, //13 left 3
	{BF_ArcMain_Left0, { 88,120, 87,117}, {132,162}}, //14 left 4
	{BF_ArcMain_Left1, {  0,  0, 87,117}, {132,162}}, //15 left 5
	{BF_ArcMain_Left1, { 87,  0, 87,116}, {132,161}}, //16 left 6
	{BF_ArcMain_Left1, {  0,117, 86,117}, {131,162}}, //17 left 7
	{BF_ArcMain_Left1, { 86,117, 86,117}, {131,162}}, //18 left 8
	{BF_ArcMain_Left2, {  0,  0, 86,117}, {131,162}}, //19 left 9
	{BF_ArcMain_Left2, { 86,  0, 86,117}, {131,162}}, //20 left 10
	{BF_ArcMain_Left2, {  0,117, 86,117}, {131,162}}, //21 left 11

	{BF_ArcMain_Leftmiss0, {  0,  0, 88,117}, {133,163}}, //22 leftmiss 1
	{BF_ArcMain_Leftmiss0, { 88,  0, 87,117}, {132,163}}, //23 leftmiss 2
	{BF_ArcMain_Leftmiss0, {  0,117, 86,115}, {131,161}}, //24 leftmiss 3
	{BF_ArcMain_Leftmiss0, { 86,117, 85,114}, {130,160}}, //25 leftmiss 4
	{BF_ArcMain_Leftmiss1, {  0,  0, 85,114}, {130,160}}, //26 leftmiss 5
	{BF_ArcMain_Leftmiss1, { 85,  0, 84,114}, {129,160}}, //27 leftmiss 6
	{BF_ArcMain_Leftmiss1, {169,  0, 84,114}, {130,160}}, //28 leftmiss 7
	{BF_ArcMain_Leftmiss1, {  0,114, 85,114}, {130,160}}, //29 leftmiss 8
	{BF_ArcMain_Leftmiss1, { 85,114, 84,114}, {129,160}}, //30 leftmiss 9
	{BF_ArcMain_Leftmiss1, {169,114, 84,113}, {130,159}}, //31 leftmiss 10
	{BF_ArcMain_Leftmiss2, {  0,  0, 84,114}, {130,160}}, //32 leftmiss 11

	{BF_ArcMain_Down0, {  0,  0, 78, 97}, {122,143}}, //33 down 1
	{BF_ArcMain_Down0, { 78,  0, 76,100}, {121,145}}, //34 down 2
	{BF_ArcMain_Down0, {154,  0, 75,101}, {120,146}}, //35 down 3
	{BF_ArcMain_Down0, {  0,102, 75,102}, {121,147}}, //36 down 4
	{BF_ArcMain_Down0, { 75,102, 75,102}, {120,147}}, //37 down 5
	{BF_ArcMain_Down0, {150,102, 75,102}, {120,147}}, //38 down 6
	{BF_ArcMain_Down1, {  0,  0, 74,103}, {120,148}}, //39 down 7
	{BF_ArcMain_Down1, { 74,  0, 74,103}, {120,148}}, //40 down 8
	{BF_ArcMain_Down1, {148,  0, 74,103}, {120,148}}, //41 down 9
	{BF_ArcMain_Down1, {  0,103, 75,103}, {120,148}}, //42 down 10
	{BF_ArcMain_Down1, { 75,103, 74,103}, {119,148}}, //43 down 11

	{BF_ArcMain_Downmiss0, {  0,  0, 81,104}, {124,151}}, //44 downmiss 1
	{BF_ArcMain_Downmiss0, { 81,  0, 80,105}, {123,152}}, //45 downmiss 2
	{BF_ArcMain_Downmiss0, {161,  0, 79,108}, {122,154}}, //46 downmiss 3
	{BF_ArcMain_Downmiss0, {  0,108, 78,107}, {122,154}}, //47 downmiss 4
	{BF_ArcMain_Downmiss0, { 78,108, 79,108}, {122,154}}, //48 downmiss 5
	{BF_ArcMain_Downmiss0, {157,108, 79,108}, {122,154}}, //49 downmiss 6
	{BF_ArcMain_Downmiss1, {  0,  0, 78,108}, {121,154}}, //50 downmiss 7
	{BF_ArcMain_Downmiss1, { 78,  0, 78,108}, {121,154}}, //51 downmiss 8
	{BF_ArcMain_Downmiss1, {156,  0, 78,108}, {122,154}}, //52 downmiss 9
	{BF_ArcMain_Downmiss1, {  0,108, 79,108}, {122,154}}, //53 downmiss 10
	{BF_ArcMain_Downmiss1, { 79,108, 78,108}, {121,154}}, //54 downmiss 11

	{BF_ArcMain_Up0, {  0,  0, 73,120}, {117,166}}, //55 up 1
	{BF_ArcMain_Up0, { 73,  0, 73,118}, {117,164}}, //56 up 2
	{BF_ArcMain_Up0, {146,  0, 73,117}, {117,163}}, //57 up 3
	{BF_ArcMain_Up0, {  0,120, 74,116}, {118,162}}, //58 up 4
	{BF_ArcMain_Up0, { 74,120, 73,116}, {117,162}}, //59 up 5
	{BF_ArcMain_Up0, {147,120, 74,115}, {117,161}}, //60 up 6
	{BF_ArcMain_Up1, {  0,  0, 74,115}, {118,161}}, //61 up 7
	{BF_ArcMain_Up1, { 74,  0, 74,115}, {118,161}}, //62 up 8
	{BF_ArcMain_Up1, {148,  0, 74,115}, {118,161}}, //63 up 9
	{BF_ArcMain_Up1, {  0,115, 74,115}, {117,161}}, //64 up 10
	{BF_ArcMain_Up1, { 74,115, 74,115}, {118,161}}, //65 up 11

	{BF_ArcMain_Upmiss0, {  0,  0, 72,119}, {117,165}}, //66 upmiss 1
	{BF_ArcMain_Upmiss0, { 72,  0, 73,118}, {116,164}}, //67 upmiss 2
	{BF_ArcMain_Upmiss0, {145,  0, 73,117}, {117,163}}, //68 upmiss 3
	{BF_ArcMain_Upmiss0, {  0,119, 73,115}, {116,162}}, //69 upmiss 4
	{BF_ArcMain_Upmiss0, { 73,119, 73,115}, {117,162}}, //70 upmiss 5
	{BF_ArcMain_Upmiss0, {146,119, 73,115}, {116,162}}, //71 upmiss 6
	{BF_ArcMain_Upmiss1, {  0,  0, 73,114}, {117,161}}, //72 upmiss 7
	{BF_ArcMain_Upmiss1, { 73,  0, 74,115}, {117,161}}, //73 upmiss 8
	{BF_ArcMain_Upmiss1, {147,  0, 74,114}, {117,161}}, //74 upmiss 9
	{BF_ArcMain_Upmiss1, {  0,115, 74,115}, {117,161}}, //75 upmiss 10
	{BF_ArcMain_Upmiss1, { 74,115, 74,115}, {117,161}}, //76 upmiss 11

	{BF_ArcMain_Right0, {  0,  0, 72,104}, {117,150}}, //77 right 1
	{BF_ArcMain_Right0, { 72,  0, 73,105}, {118,151}}, //78 right 2
	{BF_ArcMain_Right0, {145,  0, 75,106}, {120,152}}, //79 right 3
	{BF_ArcMain_Right0, {  0,108, 75,108}, {120,154}}, //80 right 4
	{BF_ArcMain_Right0, { 75,108, 76,108}, {121,154}}, //81 right 5
	{BF_ArcMain_Right0, {151,108, 76,108}, {121,154}}, //82 right 6
	{BF_ArcMain_Right1, {  0,  0, 76,108}, {121,154}}, //83 right 7
	{BF_ArcMain_Right1, { 76,  0, 76,108}, {121,154}}, //84 right 8
	{BF_ArcMain_Right1, {152,  0, 76,108}, {121,154}}, //85 right 9
	{BF_ArcMain_Right1, {  0,108, 76,108}, {121,154}}, //86 right 10
	{BF_ArcMain_Right1, { 76,108, 76,108}, {121,154}}, //87 right 11

	{BF_ArcMain_Rightmiss0, {  0,  0, 74,108}, {119,154}}, //88 rightmiss 1
	{BF_ArcMain_Rightmiss0, { 74,  0, 76,109}, {121,155}}, //89 rightmiss 2
	{BF_ArcMain_Rightmiss0, {150,  0, 77,111}, {122,157}}, //90 rightmiss 3
	{BF_ArcMain_Rightmiss0, {  0,112, 78,112}, {122,158}}, //91 rightmiss 4
	{BF_ArcMain_Rightmiss0, { 78,112, 78,112}, {123,158}}, //92 rightmiss 5
	{BF_ArcMain_Rightmiss0, {156,112, 78,112}, {123,158}}, //93 rightmiss 6
	{BF_ArcMain_Rightmiss1, {  0,  0, 79,112}, {123,158}}, //94 rightmiss 7
	{BF_ArcMain_Rightmiss1, { 79,  0, 79,112}, {123,158}}, //95 rightmiss 8
	{BF_ArcMain_Rightmiss1, {158,  0, 78,112}, {123,158}}, //96 rightmiss 9
	{BF_ArcMain_Rightmiss1, {  0,112, 78,112}, {123,158}}, //97 rightmiss 10
	{BF_ArcMain_Rightmiss1, { 78,112, 79,112}, {124,158}}, //98 rightmiss 11

	{BF_ArcMain_Dead0, {  0,   0, 128, 128}, { 53,  98}}, //23 dead0 0
	{BF_ArcMain_Dead0, {128,   0, 128, 128}, { 53,  98}}, //24 dead0 1
	{BF_ArcMain_Dead0, {  0, 128, 128, 128}, { 53,  98}}, //25 dead0 2
	{BF_ArcMain_Dead0, {128, 128, 128, 128}, { 53,  98}}, //26 dead0 3
	
	{BF_ArcDead_Dead1, {  0,   0, 128, 128}, { 53,  98}}, //27 dead1 0
	{BF_ArcDead_Dead1, {128,   0, 128, 128}, { 53,  98}}, //28 dead1 1
	{BF_ArcDead_Dead1, {  0, 128, 128, 128}, { 53,  98}}, //29 dead1 2
	{BF_ArcDead_Dead1, {128, 128, 128, 128}, { 53,  98}}, //30 dead1 3
	
	{BF_ArcDead_Dead2, {  0,   0, 128, 128}, { 53,  98}}, //31 dead2 body twitch 0
	{BF_ArcDead_Dead2, {128,   0, 128, 128}, { 53,  98}}, //32 dead2 body twitch 1
	{BF_ArcDead_Dead2, {  0, 128, 128, 128}, { 53,  98}}, //33 dead2 balls twitch 0
	{BF_ArcDead_Dead2, {128, 128, 128, 128}, { 53,  98}}, //34 dead2 balls twitch 1
};

static const Animation char_bf_anim[PlayerAnim_Max] = {
	{2, (const u8[]){ 0,  1,  2,  3,  4, 5, 6, 7, 8, 9, 10}}, //CharAnim_Idle
	{2, (const u8[]){ 11,  12, 13, 14, 15, 16},             //CharAnim_Left
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_LeftAlt
	{2, (const u8[]){ 7,  8, ASCR_BACK, 1}},             //CharAnim_Down
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_DownAlt
	{2, (const u8[]){ 9, 10, ASCR_BACK, 1}},             //CharAnim_Up
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_UpAlt
	{2, (const u8[]){11, 12, ASCR_BACK, 1}},             //CharAnim_Right
	{0, (const u8[]){ASCR_CHGANI, CharAnim_Idle}},       //CharAnim_RightAlt
	
	{1, (const u8[]){ 5, 20, 20, 21, ASCR_BACK, 1}},     //PlayerAnim_LeftMiss
	{1, (const u8[]){ 7, 22, 22, 23, ASCR_BACK, 1}},     //PlayerAnim_DownMiss
	{1, (const u8[]){ 9, 24, 24, 25, ASCR_BACK, 1}},     //PlayerAnim_UpMiss
	{1, (const u8[]){11, 26, 26, 27, ASCR_BACK, 1}},     //PlayerAnim_RightMiss
	
	{2, (const u8[]){13, 14, 15, ASCR_BACK, 1}},         //PlayerAnim_Peace
	{2, (const u8[]){16, 17, 18, 19, ASCR_REPEAT}},      //PlayerAnim_Sweat
	
	{5, (const u8[]){28, 29, 30, 31, 31, 31, 31, 31, 31, 31, ASCR_CHGANI, PlayerAnim_Dead1}}, //PlayerAnim_Dead0
	{5, (const u8[]){31, ASCR_REPEAT}},                                                       //PlayerAnim_Dead1
	{3, (const u8[]){32, 33, 34, 35, 35, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}}, //PlayerAnim_Dead2
	{3, (const u8[]){35, ASCR_REPEAT}},                                                       //PlayerAnim_Dead3
	{3, (const u8[]){36, 37, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead4
	{3, (const u8[]){38, 39, 35, 35, 35, 35, 35, ASCR_CHGANI, PlayerAnim_Dead3}},             //PlayerAnim_Dead5
	
	{10, (const u8[]){35, 35, 35, ASCR_BACK, 1}}, //PlayerAnim_Dead4
	{ 3, (const u8[]){38, 39, 35, ASCR_REPEAT}},  //PlayerAnim_Dead5
};

//Boyfriend player functions
void Char_BF_SetFrame(void *user, u8 frame)
{
	Char_BF *this = (Char_BF*)user;
	
	//Check if this is a new frame
	if (frame != this->frame)
	{
		//Check if new art shall be loaded
		const CharFrame *cframe = &char_bf_frame[this->frame = frame];
		if (cframe->tex != this->tex_id)
			Gfx_LoadTex(&this->tex, this->arc_ptr[this->tex_id = cframe->tex], 0);
	}
}

void Char_BF_Tick(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Secret icon
	if (secret)
		this->character.health_i = 3;
	else
		this->character.health_i = 0;

	if (pad_state.press & PAD_SELECT && stage.prefs.debug == 0)
		secret ++;
	
	if (secret == 2)
		secret = 0;
	//Handle animation updates
	if ((character->pad_held & (INPUT_LEFT | INPUT_DOWN | INPUT_UP | INPUT_RIGHT)) == 0 ||
	    (character->animatable.anim != CharAnim_Left &&
	     character->animatable.anim != CharAnim_LeftAlt &&
	     character->animatable.anim != CharAnim_Down &&
	     character->animatable.anim != CharAnim_DownAlt &&
	     character->animatable.anim != CharAnim_Up &&
	     character->animatable.anim != CharAnim_UpAlt &&
	     character->animatable.anim != CharAnim_Right &&
	     character->animatable.anim != CharAnim_RightAlt))
		Character_CheckEndSing(character);
	
	if (stage.flag & STAGE_FLAG_JUST_STEP)
	{
		//Perform idle dance
		if (Animatable_Ended(&character->animatable) &&
			(character->animatable.anim != CharAnim_Left &&
		     character->animatable.anim != CharAnim_LeftAlt &&
		     character->animatable.anim != PlayerAnim_LeftMiss &&
		     character->animatable.anim != CharAnim_Down &&
		     character->animatable.anim != CharAnim_DownAlt &&
		     character->animatable.anim != PlayerAnim_DownMiss &&
		     character->animatable.anim != CharAnim_Up &&
		     character->animatable.anim != CharAnim_UpAlt &&
		     character->animatable.anim != PlayerAnim_UpMiss &&
		     character->animatable.anim != CharAnim_Right &&
		     character->animatable.anim != CharAnim_RightAlt &&
		     character->animatable.anim != PlayerAnim_RightMiss) &&
			(stage.song_step & 0x7) == 0)
			character->set_anim(character, CharAnim_Idle);
	}
	
	//Retry screen
	if (character->animatable.anim >= PlayerAnim_Dead3)
	{
		//Tick skull fragments
		if (this->skull_scale)
		{
			SkullFragment *frag = this->skull;
			for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
			{
				//Draw fragment
				RECT frag_src = {
					(i & 1) ? 112 : 96,
					(i >> 1) << 4,
					16,
					16
				};
				fixed_t skull_dim = (FIXED_DEC(16,1) * this->skull_scale) >> 6;
				fixed_t skull_rad = skull_dim >> 1;
				RECT_FIXED frag_dst = {
					character->x + (((fixed_t)frag->x << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.x,
					character->y + (((fixed_t)frag->y << FIXED_SHIFT) >> 3) - skull_rad - stage.camera.y,
					skull_dim,
					skull_dim,
				};
				Stage_DrawTex(&this->tex_retry, &frag_src, &frag_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
				
				//Move fragment
				frag->x += frag->xsp;
				frag->y += ++frag->ysp;
			}
			
			//Decrease scale
			this->skull_scale--;
		}
		
		//Draw input options
		u8 input_scale = 16 - this->skull_scale;
		if (input_scale > 16)
			input_scale = 0;
		
		RECT button_src = {
			 0, 96,
			16, 16
		};
		RECT_FIXED button_dst = {
			character->x - FIXED_DEC(32,1) - stage.camera.x,
			character->y - FIXED_DEC(88,1) - stage.camera.y,
			(FIXED_DEC(16,1) * input_scale) >> 4,
			FIXED_DEC(16,1),
		};
		
		//Cross - Retry
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Circle - Blueball
		button_src.x = 16;
		button_dst.y += FIXED_DEC(56,1);
		Stage_DrawTex(&this->tex_retry, &button_src, &button_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
		
		//Draw 'RETRY'
		u8 retry_frame;
		
		if (character->animatable.anim == PlayerAnim_Dead6)
		{
			//Selected retry
			retry_frame = 2 - (this->retry_bump >> 3);
			if (retry_frame >= 3)
				retry_frame = 0;
			if (this->retry_bump & 2)
				retry_frame += 3;
			
			if (++this->retry_bump == 0xFF)
				this->retry_bump = 0xFD;
		}
		else
		{
			//Idle
			retry_frame = 1 +  (this->retry_bump >> 2);
			if (retry_frame >= 3)
				retry_frame = 0;
			
			if (++this->retry_bump >= 55)
				this->retry_bump = 0;
		}
		
		RECT retry_src = {
			(retry_frame & 1) ? 48 : 0,
			(retry_frame >> 1) << 5,
			48,
			32
		};
		RECT_FIXED retry_dst = {
			character->x -  FIXED_DEC(7,1) - stage.camera.x,
			character->y - FIXED_DEC(92,1) - stage.camera.y,
			FIXED_DEC(48,1),
			FIXED_DEC(32,1),
		};
		Stage_DrawTex(&this->tex_retry, &retry_src, &retry_dst, FIXED_MUL(stage.camera.zoom, stage.bump));
	}
	
	//Animate and draw character
	Animatable_Animate(&character->animatable, (void*)this, Char_BF_SetFrame);
	Character_Draw(character, &this->tex, &char_bf_frame[this->frame]);
}

void Char_BF_SetAnim(Character *character, u8 anim)
{
	Char_BF *this = (Char_BF*)character;
	
	//Perform animation checks
	switch (anim)
	{
		case PlayerAnim_Dead0:
			//Begin reading dead.arc and adjust focus
			this->arc_dead = IO_AsyncReadFile(&this->file_dead_arc);
			character->focus_x = FIXED_DEC(0,1);
			character->focus_y = FIXED_DEC(-40,1);
			character->focus_zoom = FIXED_DEC(125,100);
			break;
		case PlayerAnim_Dead2:
			//Unload main.arc
			Mem_Free(this->arc_main);
			this->arc_main = this->arc_dead;
			this->arc_dead = NULL;
			
			//Find dead.arc files
			const char **pathp = (const char *[]){
				"dead1.tim", //BF_ArcDead_Dead1
				"dead2.tim", //BF_ArcDead_Dead2
				"retry.tim", //BF_ArcDead_Retry
				NULL
			};
			IO_Data *arc_ptr = this->arc_ptr;
			for (; *pathp != NULL; pathp++)
				*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
			
			//Load retry art
			Gfx_LoadTex(&this->tex_retry, this->arc_ptr[BF_ArcDead_Retry], 0);
			break;
	}
	
	//Set animation
	Animatable_SetAnim(&character->animatable, anim);
	Character_CheckStartSing(character);
}

void Char_BF_Free(Character *character)
{
	Char_BF *this = (Char_BF*)character;
	
	//Free art
	Mem_Free(this->arc_main);
	Mem_Free(this->arc_dead);
}

Character *Char_BF_New(fixed_t x, fixed_t y)
{
	//Allocate boyfriend object
	Char_BF *this = Mem_Alloc(sizeof(Char_BF));
	if (this == NULL)
	{
		sprintf(error_msg, "[Char_BF_New] Failed to allocate boyfriend object");
		ErrorLock();
		return NULL;
	}
	
	//Initialize character
	this->character.tick = Char_BF_Tick;
	this->character.set_anim = Char_BF_SetAnim;
	this->character.free = Char_BF_Free;
	
	Animatable_Init(&this->character.animatable, char_bf_anim);
	Character_Init((Character*)this, x, y);
	
	//Set character information
	this->character.spec = CHAR_SPEC_MISSANIM;

	//health bar color
	this->character.health_bar = 0xFF29B5D6;
	
	this->character.focus_x = FIXED_DEC(-50,1);
	this->character.focus_y = (stage.stage_id == StageId_1_4) ? FIXED_DEC(-85,1) : FIXED_DEC(-65,1);
	this->character.focus_zoom = FIXED_DEC(12,10);
	
	//Load art
	if (stage.stage_id >= StageId_5_1 && stage.stage_id <= StageId_5_3)
	{
		this->arc_main = IO_Read("\\CHAR\\XMASBF.ARC;1");
		this->arc_dead = NULL;
		IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
		
		const char **pathp = (const char *[]){
			"xmasbf0.tim",   //BF_ArcMain_BF0
			"xmasbf1.tim",   //BF_ArcMain_BF1
			"xmasbf2.tim",   //BF_ArcMain_BF2
			"xmasbf3.tim",   //BF_ArcMain_BF3
			"xmasbf3.tim",   //BF_ArcMain_BF4
			"xmasbf4.tim",   //BF_ArcMain_BF5
			"xmasbf5.tim",   //BF_ArcMain_BF6
			"dead0.tim", //BF_ArcMain_Dead0
			NULL
		};
		IO_Data *arc_ptr = this->arc_ptr;
		for (; *pathp != NULL; pathp++)
			*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	}
	else
	{
		this->arc_main = IO_Read("\\CHAR\\BF.ARC;1");
		this->arc_dead = NULL;
		IO_FindFile(&this->file_dead_arc, "\\CHAR\\BFDEAD.ARC;1");
		
		const char **pathp = (const char *[]){
			"idle0.tim",
			"idle1.tim",
			"idle2.tim",
			"left0.tim",
			"left1.tim",
			"left2.tim",
			"leftmiss0.tim",
			"leftmiss1.tim",
			"leftmiss2.tim",
			"down0.tim",
			"down1.tim",
			"downmiss0.tim",
			"downmiss1.tim",
			"up0.tim",
			"up1.tim",
			"upmiss0.tim",
			"upmiss1.tim",
			"right0.tim",
			"right1.tim",
			"rightmiss0.tim",
			"rightmiss1.tim",
			"dead0.tim", //BF_ArcMain_Dead0
			NULL
		};
		IO_Data *arc_ptr = this->arc_ptr;
		for (; *pathp != NULL; pathp++)
			*arc_ptr++ = Archive_Find(this->arc_main, *pathp);
	}
	//Initialize render state
	this->tex_id = this->frame = 0xFF;
	
	//Initialize player state
	this->retry_bump = 0;
	
	//Copy skull fragments
	memcpy(this->skull, char_bf_skull, sizeof(char_bf_skull));
	this->skull_scale = 64;
	
	SkullFragment *frag = this->skull;
	for (size_t i = 0; i < COUNT_OF_MEMBER(Char_BF, skull); i++, frag++)
	{
		//Randomize trajectory
		frag->xsp += RandomRange(-4, 4);
		frag->ysp += RandomRange(-2, 2);
	}
	
	return (Character*)this;
}
