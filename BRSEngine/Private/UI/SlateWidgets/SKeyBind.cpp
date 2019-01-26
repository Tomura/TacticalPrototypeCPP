#include "BRSEngine.h"
#include "SlateWidgets/SKeyBind.h"
#include "CoreTypes.h"


FReply SKeyBind::OnMouseButtonDown(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	if (bWaitingForKey)
	{
		SetKey(MouseEvent.GetEffectingButton());
		return FReply::Handled();
	}
	else if (MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton)
	{
		//Get the center of the widget ao we can lock our mouse there
		FSlateRect Rect = MyGeometry.GetLayoutBoundingRect();
		WaitingMousePos.X = (Rect.Left + Rect.Right) * 0.5f;
		WaitingMousePos.Y = (Rect.Top + Rect.Bottom) * 0.5f;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->SetPosition(WaitingMousePos.X, WaitingMousePos.Y);

		//RECT Bounds;

		//Bounds.left = FMath::RoundToInt((Rect.Left - Rect.Right)*0.25f + WaitingMousePos.X);
		//Bounds.top = FMath::RoundToInt((Rect.Top - Rect.Bottom)*0.25f + WaitingMousePos.Y);
		//Bounds.right = FMath::RoundToInt((Rect.Right - Rect.Left)*0.25f + WaitingMousePos.X);
		//Bounds.bottom = FMath::RoundToInt((Rect.Bottom - Rect.Top)*0.25f + WaitingMousePos.Y);

		//FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Lock(&Bounds);

		KeyText->SetText(NSLOCTEXT("SKeyBind", "PressAnyKey", "*** Press Any Button ***"));
		bWaitingForKey = true;
		FSlateApplication::Get().GetPlatformApplication().Get()->Cursor->Show(false);
		return FReply::Handled();
	}
	return FReply::Unhandled();
}