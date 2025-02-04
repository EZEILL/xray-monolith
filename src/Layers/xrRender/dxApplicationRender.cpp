#include "stdafx.h"

#include "dxApplicationRender.h"
#include "../../xrEngine/GameFont.h"
#include "loadscreen_cfg.h"

void draw_multiline_text(CGameFont* F, float fTargetWidth, LPCSTR pszText);

void dxApplicationRender::Copy(IApplicationRender& _in)
{
	*this = *(dxApplicationRender*)&_in;
}

void dxApplicationRender::LoadBegin()
{
	ll_hGeom.create(FVF::F_TL, RCache.Vertex.Buffer(), RCache.QuadIB);
	sh_progress.create("hud\\default", "ui\\ui_actor_loadgame_screen");
	hLevelLogo_Add.create("hud\\default", "ui\\ui_actor_widescreen_sidepanels.dds");

	ll_hGeom2.create(FVF::F_TL, RCache.Vertex.Buffer(),NULL);

	//Discord
	discord_gameinfo.loadscreen = true;

	if (psDeviceFlags2.test(rsDiscord))
		updateDiscordPresence();
}

void dxApplicationRender::destroy_loading_shaders()
{
	hLevelLogo.destroy();
	sh_progress.destroy();
	hLevelLogo_Add.destroy();

	//Discord
	discord_gameinfo.loadscreen = false;

	if (psDeviceFlags2.test(rsDiscord))
	{
		discord_gameinfo.ex_update = true;
		updateDiscordPresence();
	}
}

void dxApplicationRender::setLevelLogo(LPCSTR pszLogoName)
{
	hLevelLogo.create("hud\\default", pszLogoName);
}

void dxApplicationRender::KillHW()
{
	ZeroMemory(&HW, sizeof(CHW));
}

u32 calc_progress_color(u32, u32, int, int);
extern bool use_reshade;
extern void render_reshade_effects();

cfg_var vars;
void dxApplicationRender::load_draw_internal(CApplication& owner)
{
#if defined(USE_DX10) || defined(USE_DX11)
	//	TODO: DX10: remove this???
	RImplementation.rmNormal();
	RCache.set_RT(HW.pBaseRT);
	RCache.set_ZB(HW.pBaseZB);
#endif	//	USE_DX10

#if defined(USE_DX10) || defined(USE_DX11)
	FLOAT ColorRGBA[4] = {0.0f, 0.0f, 0.0f, 0.0f};
	HW.pContext->ClearRenderTargetView(RCache.get_RT(), ColorRGBA);
#else	//	USE_DX10
	CHK_DX(HW.pDevice->Clear(0,0,D3DCLEAR_TARGET,D3DCOLOR_ARGB(0,0,0,0),1,0));
#endif	//	USE_DX10

	if (use_reshade)
		render_reshade_effects();

	if (!sh_progress)
	{
		return;
	}

#if defined(USE_DX10) || defined(USE_DX11)
	//	TODO: DX10: remove this
	//	FLOAT ColorRGBA[4] = {0.0f, 0.0f, 1.0f, 0.0f};
	//	HW.pContext->ClearRenderTargetView( RCache.get_RT(), ColorRGBA);
	//	HW.pContext->ClearDepthStencilView( RCache.get_ZB(), D3D_CLEAR_DEPTH|D3D_CLEAR_STENCIL, 1.0f, 0);
#endif	//	USE_DX10

	float _w = (float)Device.dwWidth;
	float _h = (float)Device.dwHeight;
	bool b_ws = (_w / _h) > 1.34f;
	bool b_16x9 = b_ws && ((_w / _h) > 1.77f);
	float ws_k = (b_16x9) ? 0.75f : 0.8333f; //16:9 or 16:10
	float ws_w = b_ws ? (b_16x9 ? 171.0f : 102.6f) : 0.0f;

	float bw = 1024.0f;
	float bh = 768.0f;


	//NEW----------------------------------//
	if (vars.read_once) {
		vars.has_cfg = cfg_read(&vars);
		vars.read_once = false;
		//printf(
		//	"vars.bw					%f\n"
		//	"vars.bh					%f\n"
		//	"vars.back_tex_size_x		%f\n"
		//	"vars.back_tex_size_y		%f\n"
		//	"vars.back_size_x			%f\n"
		//	"vars.back_size_y			%f\n"
		//	"vars.offs					%f\n"
		//	"vars.back_tex_coords_lt_x	%f\n"
		//	"vars.back_tex_coords_lt_y	%f\n"
		//	"vars.back_coords_lt_x		%f\n"
		//	"vars.back_coords_lt_y		%f\n"
		//	"vars.back_tex_size_x2		%f\n"
		//	"vars.back_tex_size_y2		%f\n"
		//	"vars.back_size_x2			%f\n"
		//	"vars.back_size_y2			%f\n"
		//	,
		//	vars.bw,
		//	vars.bh,
		//	vars.back_tex_size_x,
		//	vars.back_tex_size_y,
		//	vars.back_size_x,
		//	vars.back_size_y,
		//	vars.offs,
		//	vars.back_tex_coords_lt_x,
		//	vars.back_tex_coords_lt_y,
		//	vars.back_coords_lt_x,
		//	vars.back_coords_lt_y,
		//	vars.back_tex_size_x2,
		//	vars.back_tex_size_y2,
		//	vars.back_size_x2,
		//	vars.back_size_y2
		//	);
	}
	if (vars.has_cfg) {
		bw = vars.bw;
		bh = vars.bh;
		//printf("vars.bw: %f vars.bh: %f\n", vars.bw, vars.bh);
		//printf("bw: %f bh: %f\n", bw, bh);
	}
	else {
		if (vars.write_once) {
			vars.bw = bw;
			vars.bh = bh;
		}
	}
	//END NEW------------------------------//



	Fvector2 k;
	k.set(_w / bw, _h / bh);

	Fvector2 tsz;
	tsz.set(1024, 1024);
	Frect back_tex_coords;
	Frect back_coords;
	Fvector2 back_size;
	Fvector2 back_tex_size;

	static float offs = -0.5f;

	Fvector2 back_offset;

	if (b_ws)
		back_offset.set(ws_w * ws_k, 0.0f); //ws_w == 171
	else
		back_offset.set(0.0f, 0.0f);

	//progress bar
	back_tex_size.set(506, 4);
	back_size.set(506, 4);

	//NEW----------------------------------//
	if (vars.has_cfg) {
		back_tex_size.set(vars.back_tex_size_x, vars.back_tex_size_y);
		back_size.set(vars.back_size_x, vars.back_size_y);
		offs = vars.offs;
		//back_offset.set(vars.back_offset_x, vars.back_offset_y);

		//printf(
		//	"vars.back_tex_size_x		%f\n"
		//	"vars.back_tex_size_y		%f\n"
		//	"vars.back_size_x			%f\n"
		//	"vars.back_size_y			%f\n"
		//	"vars.offs					%f\n"
		//	,
		//	vars.back_tex_size_x,
		//	vars.back_tex_size_y,
		//	vars.back_size_x,
		//	vars.back_size_y,
		//	vars.offs
		//);

	}
	else {
		if (vars.write_once) {
			vars.back_tex_size_x = back_tex_size.x;
			vars.back_tex_size_y = back_tex_size.y;
			vars.back_size_x = back_size.x;
			vars.back_size_y = back_size.y;
			vars.offs = offs;
			//vars.back_offset_x = back_offset.x;
			//vars.back_offset_y = back_offset.y;

			//printf("offs: %f vars.offs: %f\n", offs, vars.offs);
		}
	}
	//END NEW------------------------------//



	if (b_ws)
		back_size.x *= ws_k; //ws

	back_tex_coords.lt.set(0, 772);

	//NEW----------------------------------//
	if (vars.has_cfg) {
		back_tex_coords.lt.set(vars.back_tex_coords_lt_x,
			vars.back_tex_coords_lt_y);
	}
	else {
		if (vars.write_once) {
			vars.back_tex_coords_lt_x = back_tex_coords.lt.x;
			vars.back_tex_coords_lt_y = back_tex_coords.lt.y;
		}
	}
	//END NEW------------------------------//

	back_tex_coords.rb.add(back_tex_coords.lt, back_tex_size);

	back_coords.lt.set(260, 599);

	//NEW----------------------------------//
	if (vars.has_cfg) {
		back_coords.lt.set(vars.back_coords_lt_x,
			vars.back_coords_lt_y);
	}
	else {
		if (vars.write_once) {
			vars.back_coords_lt_x = back_coords.lt.x;
			vars.back_coords_lt_y = back_coords.lt.y;
		}
	}
	//END NEW------------------------------//

	if (b_ws)
		back_coords.lt.x *= ws_k;
	back_coords.lt.add(back_offset);

	back_coords.rb.add(back_coords.lt, back_size);
	back_coords.lt.mul(k);
	back_coords.rb.mul(k);

	back_tex_coords.lt.x /= tsz.x;
	back_tex_coords.lt.y /= tsz.y;
	back_tex_coords.rb.x /= tsz.x;
	back_tex_coords.rb.y /= tsz.y;

	u32 Offset;
	u32 C = 0xffffffff;
	FVF::TL* pv = NULL;
	u32 v_cnt = 40;

	//NEW----------------------------------//
	if (vars.has_cfg) {
		v_cnt = vars.v_cnt;
		//printf("v_cnt: %d vars.v_cnt: %d\n", v_cnt, vars.v_cnt);
	}
	else {
		if (vars.write_once) {
			vars.v_cnt = v_cnt;
		}
	}
	//END NEW------------------------------//

	pv = (FVF::TL*)RCache.Vertex.Lock(2 * (v_cnt + 1), ll_hGeom2.stride(), Offset);
	FVF::TL* _pv = pv;
	float pos_delta = back_coords.width() / v_cnt;
	float tc_delta = back_tex_coords.width() / v_cnt;
	u32 clr = C;

	for (u32 idx = 0; idx < v_cnt + 1; ++idx)
	{
		clr = calc_progress_color(idx, v_cnt, owner.load_stage, owner.max_load_stage);

		pv->set(back_coords.lt.x + pos_delta * idx + offs, back_coords.rb.y + offs, 0 + EPS_S, 1, clr,
			back_tex_coords.lt.x + tc_delta * idx, back_tex_coords.rb.y);
		pv++;
		pv->set(back_coords.lt.x + pos_delta * idx + offs, back_coords.lt.y + offs, 0 + EPS_S, 1, clr,
			back_tex_coords.lt.x + tc_delta * idx, back_tex_coords.lt.y);
		pv++;
	}
	VERIFY(u32(pv-_pv)==2*(v_cnt+1));
	RCache.Vertex.Unlock(2 * (v_cnt + 1), ll_hGeom2.stride());

	RCache.set_Shader(sh_progress);
	RCache.set_Geometry(ll_hGeom2);
	RCache.Render(D3DPT_TRIANGLESTRIP, Offset, 2 * v_cnt);

	//background picture

	back_tex_size.set(1024, 768);
	back_size.set(1024, 768);

	//NEW----------------------------------//
	if (vars.has_cfg) {
		back_tex_size.set(	vars.back_tex_size_x2,
							vars.back_tex_size_y2);
		back_size.set(	vars.back_size_x2,
						vars.back_size_y2);
		//printf(
		//	"vars.back_tex_size_x2		%f\n"
		//	"vars.back_tex_size_y2		%f\n"
		//	"vars.back_size_x2			%f\n"
		//	"vars.back_size_y2			%f\n"
		//	,
		//	vars.back_tex_size_x2,
		//	vars.back_tex_size_y2,
		//	vars.back_size_x2,
		//	vars.back_size_y2
		//);
	}
	else {
		if (vars.write_once) {
			vars.back_tex_size_x2 = back_tex_size.x;
			vars.back_tex_size_y2 = back_tex_size.y;
			vars.back_size_x2 = back_size.x;
			vars.back_size_y2 = back_size.y;
			//printf("back_size.x: %f back_size.y: %f\n", back_size.x, back_size.y);
			//printf("back_size_x2.x: %f back_size_x2.y: %f\n", vars.back_size_x2, vars.back_size_y2);
		}
	}
	//END NEW------------------------------//

	if (b_ws)
		back_size.x *= ws_k; //ws

	back_tex_coords.lt.set(0, 0);
	back_tex_coords.rb.add(back_tex_coords.lt, back_tex_size);

	back_coords.lt.set(offs, offs);
	back_coords.lt.add(back_offset);
	back_coords.rb.add(back_coords.lt, back_size);

	back_coords.lt.mul(k);
	back_coords.rb.mul(k);
	draw_face(sh_progress, back_coords, back_tex_coords, tsz);

	if (b_ws) //draw additional frames (left&right)
	{
		//left
		back_size.set(ws_w * ws_k, 768.0f);

		if (b_16x9)
		{
			back_tex_coords.lt.set(0, 0);
			back_tex_coords.rb.set(128, 768);
		}
		else
		{
			back_tex_coords.lt.set(0, 0);
			back_tex_coords.rb.set(128, 768);
		}

		//NEW----------------------------------//
		if (vars.has_cfg) {
			back_size.set(ws_w* ws_k, vars.back_size_y3);
			back_tex_coords.rb.set(	vars.back_tex_coords_rb_x,
									vars.back_tex_coords_rb_y);
			//printf(
			//	"back_size.y: %f\n"
			//	"back_tex_coords.rb.x: %f\n"
			//	"back_tex_coords.rb.y: %f\n"
			//	,
			//	back_size.y,
			//	back_tex_coords.rb.x,
			//	back_tex_coords.rb.y
			//);
		}
		else {
			if (vars.write_once) {
				vars.back_tex_coords_rb_x = back_tex_coords.rb.x;
				vars.back_tex_coords_rb_y = back_tex_coords.rb.y;
				vars.back_size_y3 = back_size.y;
			}
		}
		//END NEW------------------------------//


		back_coords.lt.set(offs, offs);
		back_coords.rb.add(back_coords.lt, back_size);
		back_coords.lt.mul(k);
		back_coords.rb.mul(k);

		draw_face(hLevelLogo_Add, back_coords, back_tex_coords, tsz);

		//right
		if (b_16x9)
		{
			back_tex_coords.lt.set(128, 0);
			back_tex_coords.rb.set(256, 768);
		}
		else
		{
			back_tex_coords.lt.set(128, 0);
			back_tex_coords.rb.set(256, 768);
		}

		//NEW----------------------------------//
		if (vars.has_cfg) {
			back_tex_coords.lt.set(	vars.back_tex_coords_lt_x2,
									vars.back_tex_coords_lt_y2);
			back_tex_coords.rb.set(	vars.back_tex_coords_rb_x2,
									vars.back_tex_coords_rb_y2);
			//printf(
			//	"back_tex_coords.lt.x: %f\n"
			//	"back_tex_coords.lt.y: %f\n"
			//	"back_tex_coords.rb.x: %f\n"
			//	"back_tex_coords.rb.y: %f\n"
			//	,
			//	back_tex_coords.lt.x,
			//	back_tex_coords.lt.y,
			//	back_tex_coords.rb.x,
			//	back_tex_coords.rb.y
			//);
		}
		else {
			if (vars.write_once) {
				vars.back_tex_coords_lt_x2 = back_tex_coords.lt.x;
				vars.back_tex_coords_lt_y2 = back_tex_coords.lt.y;
				vars.back_tex_coords_rb_x2 = back_tex_coords.rb.x;
				vars.back_tex_coords_rb_y2 = back_tex_coords.rb.y;
				//printf(
				//	"back_tex_coords.lt.x: %f\n"
				//	"back_tex_coords.lt.y: %f\n"
				//	,
				//	back_tex_coords.lt.x,
				//	back_tex_coords.lt.y
				//);
			}
		}
		//END NEW------------------------------//

		back_coords.lt.set(1024.0f - back_size.x + offs, offs);
		back_coords.rb.add(back_coords.lt, back_size);
		back_coords.lt.mul(k);
		back_coords.rb.mul(k);

		draw_face(hLevelLogo_Add, back_coords, back_tex_coords, tsz);
	}



	// Draw title
	VERIFY(owner.pFontSystem);
	owner.pFontSystem->Clear();
	owner.pFontSystem->SetColor(color_rgba(103, 103, 103, 255));
	owner.pFontSystem->SetAligment(CGameFont::alCenter);
	back_size.set(_w / 2, 622.0f * k.y);
	owner.pFontSystem->OutSet(back_size.x, back_size.y);
	owner.pFontSystem->OutNext(owner.ls_header);
	owner.pFontSystem->OutNext("");
	owner.pFontSystem->OutNext(owner.ls_tip_number);

	float fTargetWidth = 600.0f * k.x * (b_ws ? 0.8f : 1.0f);
	draw_multiline_text(owner.pFontSystem, fTargetWidth, owner.ls_tip);

	owner.pFontSystem->OnRender();

	//draw level-specific screenshot
	if (hLevelLogo)
	{
		Frect r;
		r.lt.set(0, 173);

		//NEW----------------------------------//
		if (vars.has_cfg) {
			r.lt.set(vars.r_lt_x, vars.r_lt_y);
		}
		else {
			if (vars.write_once) {
				vars.r_lt_x = r.lt.x;
				vars.r_lt_y = r.lt.y;
				//printf("r.lt.x: %f r.lt.y: %f\n", r.lt.x, r.lt.y);
			}
		}
		//END NEW------------------------------//

		if (b_ws)
			r.lt.x *= ws_k;
		r.lt.add(back_offset);

		r.lt.x += offs;
		r.lt.y += offs;
		back_size.set(1024, 399);

		//NEW----------------------------------//
		if (vars.has_cfg) {
			back_size.set(vars.back_size_x4, vars.back_size_y4);
		}
		else {
			if (vars.write_once) {
				vars.back_size_x4 = back_size.x;
				vars.back_size_y4 = back_size.y;
				//printf("back_size.x: %f back_size.y: %f\n", back_size.x, back_size.y);
			}
		}
		//END NEW------------------------------//

		if (b_ws)
			back_size.x *= ws_k; //ws 0.625

		r.rb.add(r.lt, back_size);
		r.lt.mul(k);
		r.rb.mul(k);
		Frect logo_tex_coords;
		logo_tex_coords.lt.set(0, 0);
		logo_tex_coords.rb.set(1.0f, 0.77926f);

		//NEW----------------------------------//
		if (vars.has_cfg) {
			logo_tex_coords.lt.set(vars.logo_tex_coords_lt_x, vars.logo_tex_coords_lt_y);
			logo_tex_coords.rb.set(vars.logo_tex_coords_rb_x, vars.logo_tex_coords_rb_y);
		}
		else {
			if (vars.write_once) {
				vars.logo_tex_coords_lt_x = logo_tex_coords.lt.x;
				vars.logo_tex_coords_lt_y = logo_tex_coords.lt.y;
				vars.logo_tex_coords_rb_x = logo_tex_coords.rb.x;
				vars.logo_tex_coords_rb_y = logo_tex_coords.rb.y;

				//cfg_write(&vars);
				//vars.write_screenshot = false;
			}
		}
		//it takes a few frames for the engine to load a
		//level specific screenshot
		if (vars.write_once && !vars.has_cfg) {
			//printf("writing more than once?");
			cfg_write(&vars);
			vars.write_once = false;
		}
		//END NEW------------------------------//

		draw_face(hLevelLogo, r, logo_tex_coords, Fvector2().set(1, 1));
	}

	//if (vars.write_once && !vars.has_cfg) {
	//	//printf("writing more than once?");
	//	cfg_write(&vars);
	//	vars.write_once = false;
	//}
}

void dxApplicationRender::draw_face(ref_shader& sh, Frect& coords, Frect& tex_coords, const Fvector2& tsz)
{
	u32 Offset;
	u32 C = 0xffffffff;
	FVF::TL* pv = NULL;

	tex_coords.lt.x /= tsz.x;
	tex_coords.lt.y /= tsz.y;
	tex_coords.rb.x /= tsz.x;
	tex_coords.rb.y /= tsz.y;

	pv = (FVF::TL*)RCache.Vertex.Lock(4, ll_hGeom.stride(), Offset);
	pv->set(coords.lt.x, coords.rb.y, C, tex_coords.lt.x, tex_coords.rb.y);
	pv++;
	pv->set(coords.lt.x, coords.lt.y, C, tex_coords.lt.x, tex_coords.lt.y);
	pv++;
	pv->set(coords.rb.x, coords.rb.y, C, tex_coords.rb.x, tex_coords.rb.y);
	pv++;
	pv->set(coords.rb.x, coords.lt.y, C, tex_coords.rb.x, tex_coords.lt.y);
	pv++;
	RCache.Vertex.Unlock(4, ll_hGeom.stride());

	RCache.set_Shader(sh);
	RCache.set_Geometry(ll_hGeom);
	RCache.Render(D3DPT_TRIANGLELIST, Offset, 0, 4, 0, 2);
}

u32 calc_progress_color(u32 idx, u32 total, int stage, int max_stage)
{
	float kk = (float(stage + 1) / float(max_stage)) * (total);
	float f = 1 / (exp((float(idx) - kk) * 0.5f) + 1.0f);

	float r = 1.0f;
	float g = 1.0f;
	float b = 1.0f;

	if (vars.has_cfg) {
		r = vars.r;
		g = vars.g;
		b = vars.b;
		//printf("r: %f g: %f b: %f\n", r,g,b);
	}
	else {
		if (vars.write_once) {
			vars.r = r;
			vars.g = g;
			vars.b = b;
		}
	}

	return color_argb_f(f, r, g, b); 
}

#define IsSpace(ch)       ((ch) == ' ' || (ch) == '\t' || (ch) == '\r' || (ch) == '\n' || (ch) == ',' || (ch) == '.' || (ch) == ':' || (ch) == '!')

void parse_word(LPCSTR str, CGameFont* font, float& length, LPCSTR& next_word)
{
	length = 0.0f;
	while (*str && !IsSpace(*str))
	{
		//		length  += font->GetCharTC(*str).z;
		length += font->SizeOf_(*str);
		++str;
	}
	next_word = (*str) ? str + 1 : str;
}

void draw_multiline_text(CGameFont* F, float fTargetWidth, LPCSTR pszText)
{
	if (!pszText || xr_strlen(pszText) == 0)
		return;

	LPCSTR ch = pszText;
	float curr_word_len = 0.0f;
	LPCSTR next_word = NULL;

	float curr_len = 0.0f;
	string512 buff;
	buff[0] = 0;
	while (*ch)
	{
		parse_word(ch, F, curr_word_len, next_word);
		if (curr_len + curr_word_len > fTargetWidth)
		{
			F->OutNext(buff);
			curr_len = 0.0f;
			buff[0] = 0;
		}
		else
		{
			curr_len += curr_word_len;
			strncpy_s(buff + xr_strlen(buff), sizeof(buff) - xr_strlen(buff), ch, next_word - ch);
			ch = next_word;
		}
		if (0 == *next_word) //end of text
		{
			strncpy_s(buff + xr_strlen(buff), sizeof(buff) - xr_strlen(buff), ch, next_word - ch);
			F->OutNext(buff);
			break;
		}
	}
}
