#include <string.h>
#include <unistd.h>
#include <fat.h>

#include "libwiigui/gui.h"
#include "libwiigui/gui_customoptionbrowser.h"
#include "prompts/PromptWindows.h"
#include "language/gettext.h"
#include "fatmounter.h"
#include "menu.h"
#include "filelist.h"
#include "sys.h"
#include "gct.h"

/*** Extern functions ***/
extern void ResumeGui();
extern void HaltGui();

/*** Extern variables ***/
extern GuiWindow * mainWindow;

/****************************************************************************
 * CheatMenu
 ***************************************************************************/
int CheatMenu(const char * gameID)
{
	int choice = 0;
	bool exit = false;
	int ret = 1;

	GuiSound btnSoundOver(button_over_pcm, button_over_pcm_size, SOUND_PCM, Settings.sfxvolume);
	GuiSound btnClick(button_click2_pcm, button_click2_pcm_size, SOUND_PCM, Settings.sfxvolume);

	char imgPath[100];
	snprintf(imgPath, sizeof(imgPath), "%sbutton_dialogue_box.png", CFG.theme_path);
	GuiImageData btnOutline(imgPath, button_dialogue_box_png);
	snprintf(imgPath, sizeof(imgPath), "%ssettings_background.png", CFG.theme_path);
	GuiImageData settingsbg(imgPath, settings_background_png);

	GuiTrigger trigA;
	trigA.SetSimpleTrigger(-1, WPAD_BUTTON_A | WPAD_CLASSIC_BUTTON_A, PAD_BUTTON_A);
	GuiTrigger trigB;
	trigB.SetButtonOnlyTrigger(-1, WPAD_BUTTON_B | WPAD_CLASSIC_BUTTON_B, PAD_BUTTON_B);

	GuiImage settingsbackground(&settingsbg);

	GuiText backBtnTxt(tr("Back") , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	backBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage backBtnImg(&btnOutline);
	GuiButton backBtn(&backBtnImg,&backBtnImg, 2, 3, 160, 400, &trigA, &btnSoundOver, &btnClick,1);
	backBtn.SetLabel(&backBtnTxt);
	backBtn.SetTrigger(&trigB);

	GuiText createBtnTxt("Create" , 22, (GXColor){THEME.prompttxt_r, THEME.prompttxt_g, THEME.prompttxt_b, 255});
	createBtnTxt.SetMaxWidth(btnOutline.GetWidth()-30);
	GuiImage createBtnImg(&btnOutline);
	GuiButton createBtn(&createBtnImg,&createBtnImg, 2, 3, -140, 400, &trigA, &btnSoundOver, &btnClick,1);
	createBtn.SetLabel(&createBtnTxt);

	GCTCheats c;

	char txtfilename[30];
	snprintf(txtfilename,sizeof(txtfilename),"%s%s.txt",Settings.TxtCheatcodespath,gameID);

	int check = c.openTxtfile(txtfilename);
	switch(check)
	{
	case -1: WindowPrompt("Error","Cheatfile empty",tr("OK"),NULL,NULL,NULL);
			 break;
	case 0: WindowPrompt("Error","No Cheatfile found",tr("OK"),NULL,NULL,NULL);
			break;
	case 1:	//WindowPrompt("Opened File","File found for Game","Okay",NULL,NULL,NULL);
	int cntcheats = c.getCnt()+1;
	customOptionList options2(cntcheats);
	GuiCustomOptionBrowser optionBrowser2(400, 280, &options2, CFG.theme_path, "bg_options_settings.png", bg_options_settings_png, 0, 150);
	optionBrowser2.SetPosition(0, 90);
	optionBrowser2.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	optionBrowser2.SetClickable(true);
	optionBrowser2.SetScrollbar(1);

	GuiText titleTxt(c.getGameName().c_str(), 28, (GXColor){0, 0, 0, 255});
	titleTxt.SetAlignment(ALIGN_CENTRE, ALIGN_TOP);
	titleTxt.SetMaxWidth(350, GuiText::SCROLL);
	titleTxt.SetPosition(12,40);
	
	//options2.SetName(0, "%s",c.getCheatComment(0).c_str());
	for(int i = 0; i <= cntcheats; i++) 
	{
	options2.SetName(i+1, "%s",c.getCheatName(i).c_str());
	options2.SetValue(i+1, "OFF");
	}

	HaltGui();
	GuiWindow w(screenwidth, screenheight);
	w.Append(&settingsbackground);
	w.Append(&titleTxt);
	w.Append(&backBtn);
	w.Append(&createBtn);
	w.Append(&optionBrowser2);
	mainWindow->Append(&w);
	ResumeGui();
		
	while(!exit)
	{
	VIDEO_WaitVSync ();

	ret = optionBrowser2.GetClickedOption();
	if (ret)
	{
		const char *tt = options2.GetValue(ret);
		if (strncmp(tt,"ON",2) == 0)
		{
		options2.SetValue(ret,"%s","OFF");
		}
		else if (strncmp(tt,"OFF",2) == 0) 
		{
		options2.SetValue(ret,"%s","ON");
		}
	}

	if(createBtn.GetState() == STATE_CLICKED)
	{
		createBtn.ResetState();
		if (cntcheats > 0) 
		{
		int selectednrs[30];
		int x = 0;
		for(int i = 0; i <= cntcheats; i++) 
		{
			const char *tt = options2.GetValue(i+1);
			if (strncmp(tt,"ON",2) == 0) 
			{
			selectednrs[x] = i;
			x++;
			//WindowPrompt("Selected Cheat",c.getCheatName(i).c_str(),"Okay",NULL,NULL,NULL);
			}
		}

		string chtpath = Settings.Cheatcodespath;
		string gctfname = chtpath + c.getGameID() + ".gct";
		c.createGCT(selectednrs,x,gctfname.c_str());
		WindowPrompt("GCT File created",NULL,tr("OK"),NULL,NULL,NULL);
		exit = true;
		break;				
		} else WindowPrompt("Error","Couldn�t create GCT file",tr("OK"),NULL,NULL,NULL);
		
	}

	if(backBtn.GetState() == STATE_CLICKED)
	{
		backBtn.ResetState();
		exit = true;
		break;
	}
}
HaltGui();
mainWindow->Remove(&w);
ResumeGui();	

break;
}
		
return choice;
}