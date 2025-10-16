#include <X11/XF86keysym.h>
#include "fibonacci.c"
#include "movestack.c"
#include "selfrestart.c"

/*=====================definitions=====================*/


/* mini-monocle options */
static const int fixedwindowwidth = 1500;
static const int fixedwindowheight = 1000;

/* gaps */
static const unsigned int gappih    = 40;       /* horiz inner gap between windows */
static const unsigned int gappiv    = 20;       /* vert inner gap between windows */
static const unsigned int gappoh    = 20;       /* horiz outer gap between windows and screen edge */
static const unsigned int gappov    = 20;       /* vert outer gap between windows and screen edge */
static       int smartgaps          = 0;        /* 1 means no outer gap when there is only one window */

/* appearance */
static const int showemptytags      = 1;
static const unsigned int borderpx  = 1;        /* border pixel of windows */
static const unsigned int snap      = 10;       /* snap pixel */
static const int systraypinningfailfirst = 1;   
static const int showsystray        = 1;     
static const int showbar            = 1;        
static const int topbar             = 1;        
static const char *fonts[]          = { "monospace:size=12" };
static const char dmenufont[]       = "monospace:size=12";
static const char dmenuheight[]	    = "25";
static const char dmenucenter[]     = "";
static const char dmenulinenu[]     = "8";
static const char termtitle[]	    = "Terminal";
static const int vertpad            = 0;       
static const int sidepad            = 0;       
static const char col_gray1[]       = "#1d2021"; 
static const char col_gray2[]       = "#1d2021"; 
static const char col_gray3[]       = "#a89984";
static const char col_gray4[]       = "#ebdbb2";
static const char col_accent[]	    = "#3c3836";
static const char selbordercolor[]  = "#ebdbb2";
static const char scratchpadname[] = "Scratchpad";
static const char *scratchpadcmd[] = { "st", "-t", scratchpadname, "-g", "60%x60%", "-e", "/bin/fish", NULL };
static const char *colors[][3]      = {
	/*               fg         bg         border   */
	[SchemeNorm] = { col_gray3, col_gray1, col_gray1 },
	[SchemeSel]  = { col_gray4, col_accent,  selbordercolor  },
	[SchemeStatus]  = { col_gray3, col_gray1,  "#282828"  },
	[SchemeTagsSel]  = { col_gray4, col_accent,  "#000000"  },
    [SchemeTagsNorm]  = { col_gray3, col_gray1,  "#000000"  },
    [SchemeInfoSel]  = { col_gray4, col_accent,  "#000000"  },
    [SchemeInfoNorm]  = { col_gray3, col_gray1,  "#000000"  },
};

/* tagging */
static const char *tags[] = { "I", "II", "III", "IV", "V", "VI", "VII", "VIII", "IX" };
//static const char *tags[] = { "", "", "", "", "", "", "", "", "" };
static const char ptagf[] = "%s";	/* format of a tag label */
static const char etagf[] = "%s";	/* format of an empty tag */
static const int lcaselbl = 0;		/* 1 means make tag label lowercase */	
static const unsigned int ulinepad	= 5;	/* horizontal padding between the underline and tag */
static const unsigned int ulinestroke	= 2;	/* thickness / height of the underline */
static const unsigned int ulinevoffset	= 0;	/* how far above the bottom of the bar the line should appear */
static const int ulineall 		= 0;	/* 1 to show underline on all tags, 0 for just the active ones */

static const Rule rules[] ={
  /*xprop(1):
		WM_CLASS(STRING) = instance, class
		WM_NAME(STRING) = title*/

	/* class      instance    title       tags mask     isfloating   monitor */
{ "Gimp",     NULL,       NULL,       0,            1,           -1 },
{"Firefox",  NULL,       NULL,       0,       0,           -1 },
};

/* layout(s) */
static const float mfact     = 0.55; /* factor of master area size [0.05..0.95] */
static const int nmaster     = 1;    /* number of clients in master area */
static const int resizehints = 0;    /* 1 means respect size hints in tiled resizals */

static const Layout layouts[] = {
	/* symbol     arrange function */
	{ "[T]",      tile },    /* first entry is default */
	{ "[F]",      NULL },    /* no layout function means floating behavior */
	{ "[M]",      monocle },
    { "[m]",      mini_monocle },
	{ "|M|",      centeredmaster },
	{ "[//]",     dwindle },
};

/* key definitions */
#define MODKEY Mod4Mask
#define TAGKEYS(KEY,TAG) \
	{ MODKEY,                       KEY,      view,           {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask,           KEY,      toggleview,     {.ui = 1 << TAG} }, \
	{ MODKEY|ShiftMask,             KEY,      tag,            {.ui = 1 << TAG} }, \
	{ MODKEY|ControlMask|ShiftMask, KEY,      toggletag,      {.ui = 1 << TAG} },

/* helper for spawning shell commands in the pre dwm-5.0 fashion */
#define SHCMD(cmd) { .v = (const char*[]){ "/bin/sh", "-c", cmd, NULL } }

/* commands */
static char dmenumon[2] = "0"; /* component of dmenucmd, manipulated in spawn() */
static const char *dmenucmd[] = { 
    "dmenu_run", 
    "-m", dmenumon, 
    "-fn", dmenufont, 
    "-nb", col_gray1,    /* normal background */
    "-nf", col_gray3,    /* normal foreground */
    "-sb", col_gray4,    /* selected background */
    "-sf", col_gray1,    /* selected foreground */
    NULL 
};
static const char *termcmd[]  = { "st", "-t", termtitle, "-e", "/bin/fish", NULL };
static const char *shutdown[] = {"strat", "void", "doas", "poweroff", NULL };
static const char *upvol[]   = { "sh", "-c", "for sink in $(pactl list sinks short | awk '{print $1}'); do pactl set-sink-volume $sink +5%; done; for input in $(pactl list sink-inputs short | awk '{print $1}'); do pactl set-sink-input-volume $input +5%; done", NULL };
static const char *downvol[] = { "sh", "-c", "for sink in $(pactl list sinks short | awk '{print $1}'); do pactl set-sink-volume $sink -5%; done; for input in $(pactl list sink-inputs short | awk '{print $1}'); do pactl set-sink-input-volume $input -5%; done", NULL };
static const char *mutevol[] = { "sh", "-c", "for sink in $(pactl list sinks short | awk '{print $1}'); do pactl set-sink-mute $sink toggle; done", NULL };


/*=====================keybinds=====================*/


static Key keys[] = {
	/* modifier                     key        function        argument */
	{ 0,                            XK_Print,  spawn,          SHCMD("maim -s -u | xclip -selection clipboard -t image/png") },
    { MODKEY,                       XK_w,      spawn,          SHCMD("firefox") },
    { MODKEY,                       XK_e,      spawn,          SHCMD("thunar") },
	{ 0,                            XF86XK_AudioLowerVolume, spawn, {.v = downvol } },
	{ 0,                            XF86XK_AudioRaiseVolume, spawn, {.v = upvol } },
	{ 0,                            XF86XK_AudioMute, spawn, {.v = mutevol } },
	{ MODKEY,                       XK_F12,    spawn,          {.v = shutdown } },
	{ MODKEY,                       XK_p,      spawn,          {.v = dmenucmd } },
	{ MODKEY,                       XK_Return, spawn,          {.v = termcmd } },
	{ MODKEY,                       XK_q,      togglescratch,  {.v = scratchpadcmd } },
	{ MODKEY,                       XK_b,      togglebar,      {0} },
	{ MODKEY|ShiftMask,             XK_j,      movestack,      {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_k,      movestack,      {.i = -1 } },
	{ MODKEY,                       XK_k,      focusstack,     {.i = +1 } },
	{ MODKEY,                       XK_j,      focusstack,     {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_f,      togglefullscr,  {0} },
	{ MODKEY,                       XK_h,      setmfact,       {.f = -0.05} },
	{ MODKEY,                       XK_l,      setmfact,       {.f = +0.05} },
	{ MODKEY|ShiftMask,    		XK_Return, zoom,           {0} },
	{ MODKEY,                       XK_Tab,    view,           {0} },
	{ MODKEY,	                XK_c,      killclient,     {0} },
	{ MODKEY,                       XK_t,      setlayout,      {.v = &layouts[0]} },
	{ MODKEY,                       XK_f,      setlayout,      {.v = &layouts[1]} },
	{ MODKEY,                       XK_m,      setlayout,      {.v = &layouts[2]} },
	{ MODKEY|ShiftMask,             XK_space,  togglefloating, {0} },
	{ MODKEY|ShiftMask,             XK_0,      tag,            {.ui = ~0 } },
	{ MODKEY,                       XK_comma,  focusmon,       {.i = -1 } },
	{ MODKEY,                       XK_period, focusmon,       {.i = +1 } },
	{ MODKEY|ShiftMask,             XK_comma,  tagmon,         {.i = -1 } },
	{ MODKEY|ShiftMask,             XK_period, tagmon,         {.i = +1 } },
    { MODKEY,                       XK_o,      setlayout,      {.v = &layouts[5]} },
    { MODKEY|ShiftMask,             XK_m,      setlayout,      {.v = &layouts[3]} },
    { MODKEY|ShiftMask,             XK_r,      self_restart,   {0} },
    { MODKEY,                       XK_u,      setlayout,      {.v = &layouts[4]} },
    { MODKEY|ShiftMask,             XK_o,      setcfact,       {.f = +0.25} },
    { MODKEY|ShiftMask,             XK_i,      setcfact,       {.f = -0.25} },
    { MODKEY|ShiftMask,             XK_p,      setcfact,       {.f =  0.00} },
	TAGKEYS(                        XK_1,                      0)
	TAGKEYS(                        XK_2,                      1)
	TAGKEYS(                        XK_3,                      2)
	TAGKEYS(                        XK_4,                      3)
	TAGKEYS(                        XK_5,                      4)
	TAGKEYS(                        XK_6,                      5)
	TAGKEYS(                        XK_7,                      6)
	TAGKEYS(                        XK_8,                      7)
	TAGKEYS(                        XK_9,                      8)
    { MODKEY|ShiftMask,             XK_q,      spawn,          SHCMD("pkill dwm") },
};

/* button definitions */
/* click can be ClkTagBar, ClkLtSymbol, ClkStatusText, ClkWinTitle, ClkClientWin, or ClkRootWin */
static Button buttons[] = {
	/* click                event mask      button          function        argument */
	{ ClkLtSymbol,          0,              Button1,        setlayout,      {0} },
	{ ClkLtSymbol,          0,              Button3,        setlayout,      {.v = &layouts[2]} },
	{ ClkWinTitle,          0,              Button2,        zoom,           {0} },
	{ ClkStatusText,        0,              Button2,        spawn,          {.v = termcmd } },
	{ ClkClientWin,         MODKEY,         Button1,        movemouse,      {0} },
	{ ClkClientWin,         MODKEY,         Button2,        togglefloating, {0} },
	{ ClkClientWin,         MODKEY,         Button3,        resizemouse,    {0} },
  /*	{ ClkTagBar,            0,              Button1,        view,           {0} },*/
	{ ClkTagBar,            0,              Button3,        toggleview,     {0} },
	{ ClkTagBar,            MODKEY,         Button1,        tag,            {0} },
	{ ClkTagBar,            MODKEY,         Button3,        toggletag,      {0} },
};
