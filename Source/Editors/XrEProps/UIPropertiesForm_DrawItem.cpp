#include "stdafx.h"
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
#define TSTRING_COUNT 	3
const LPSTR TEXTUREString[TSTRING_COUNT] = { "Custom...","$null","$base0" };
template<typename T>
inline bool DrawNumeric(PropItem* item, bool& change, bool read_only)
{
	change = false;
	NumericValue<T>* V = dynamic_cast<NumericValue<T>*>(item->GetFrontValue());
	if (!V)					return false;
	int data = 0;
	{
		T value = *V->value;
		item->BeforeEdit<NumericValue<T>, T>(value);
		data = static_cast<int>(value);
	}
	change = ImGui::InputInt("##value", &data, read_only ? ImGuiInputTextFlags_ReadOnly : 0);
	if (change)
	{
	
		if (int(V->lim_mn) > data)
			data = int(V->lim_mn);
		if (int(V->lim_mx) < data)
			data = int(V->lim_mx);
		T temp = T(data);
		if ( item->AfterEdit< NumericValue<T>, T>(temp) && !read_only)
		{
			change = item->ApplyValue< NumericValue<T>, T>(temp);
		}
		return true;
	}
	
}
template<>
inline bool DrawNumeric<float>(PropItem* item, bool& change, bool read_only)
{
	change = false;
	NumericValue<float>* V = dynamic_cast<NumericValue<float>*>(item->GetFrontValue());
	if (!V)					return false;
	float temp = *V->value;
	item->BeforeEdit<NumericValue<float>, float>(temp);
	change = ImGui::InputFloat("##value", &temp, 0.01, 0.1, V->dec, read_only ? ImGuiInputTextFlags_ReadOnly : 0);
	if (change)
	{
		if (!isinf(V->lim_mn) &&V->lim_mn > temp)
			temp = V->lim_mn;
		if (!isinf(V->lim_mx) && V->lim_mx < temp)
			temp = V->lim_mx;
		if ( item->AfterEdit< NumericValue<float>, float>(temp) && !read_only)
		{
			change = item->ApplyValue< NumericValue<float>, float>(temp);
		}
	}
	return true;
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
template <class T>
BOOL TokenOnEdit(PropItem* prop, bool& change)
{
	TokenValue<T>* V = dynamic_cast<TokenValue<T>*>(prop->GetFrontValue());
	if (!V)					return FALSE;
	T edit_value  = V->GetValue();
	prop->BeforeEdit<TokenValue<T>, T>(edit_value);
	int index = edit_value;
	const char* InTokens[256];
	xr_token* token_list = V->token;
	int i = 0;
	for (; token_list[i].name; i++)
	{
		InTokens[i] = token_list[i].name;
	}

	if (ImGui::Combo("##value", &index, InTokens, i))
	{
		T new_val = index;
		if (prop->AfterEdit<TokenValue<T>, T>(new_val))
			change = prop->ApplyValue<TokenValue<T>, T>(new_val);
	}
	return TRUE;
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
template <class T>
BOOL RTokenOnEdit(PropItem* prop, bool& change)
{
	RTokenValue<T>* V = dynamic_cast<RTokenValue<T>*>(prop->GetFrontValue());
	if (!V)					return FALSE;
	T edit_value = V->GetValue();
	prop->BeforeEdit<TokenValue<T>, T>(edit_value);
	int index = edit_value;
	const char* InTokens[256];
	xr_rtoken* token_list = V->token;
	int i = 0;
	for (;i< V->token_count; i++)
	{
		InTokens[i] = token_list[i].name.c_str();
	}
	if (ImGui::Combo("##value", &index, InTokens, i))
	{
		T new_val = index;
		if (prop->AfterEdit<RTokenValue<T>, T>(new_val))
			change = prop->ApplyValue<RTokenValue<T>, T>(new_val);
	}
	return TRUE;
}
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
//-----------------------------------------------------------------------------------------
template <class T>
BOOL FlagOnEdit(PropItem* prop, bool& change)
{
	FlagValue<_flags<T> >* V = dynamic_cast<FlagValue<_flags<T> >*>(prop->GetFrontValue());
	if (!V)					return FALSE;
	_flags<T> new_val = V->GetValue();

	prop->BeforeEdit<FlagValue<_flags<T> >, _flags<T> >(new_val);
	u32 u = new_val.get();

	if (ImGui::CheckboxFlags("##value", &u, V->mask))
	{
		new_val.assign(u);
		if (prop->AfterEdit<FlagValue<_flags<T> >, _flags<T> >(new_val))
			change = prop->ApplyValue<FlagValue<_flags<T> >, _flags<T> >(new_val);
	}
	return TRUE;
}
void UIPropertiesForm::DrawItem(const char* name, PropItem* node)
{
	EPropType type = node->Type();
	switch (type)
	{
	case PROP_NUMERIC:
	{
		bool change = false;
		if (!DrawNumeric<u32>(node, change, m_Flags.test(plReadOnly)))
			if (!DrawNumeric<float>(node, change, m_Flags.test(plReadOnly)))
				if (!DrawNumeric<u8>(node, change, m_Flags.test(plReadOnly)))
					if (!DrawNumeric<s8>(node, change, m_Flags.test(plReadOnly)))
						if (!DrawNumeric<u16>(node, change, m_Flags.test(plReadOnly)))
							if (!DrawNumeric<u16>(node, change, m_Flags.test(plReadOnly)))
								if (!DrawNumeric<s32>(node, change, m_Flags.test(plReadOnly)))
									R_ASSERT(false);
		if (change)Modified();
	}
	break;
	case PROP_SHORTCUT:
	{
		ShortcutValue* V = dynamic_cast<ShortcutValue*>(node->GetFrontValue()); R_ASSERT(V);
		if (ImGui::Button(V->GetDrawText(0).c_str(), ImVec2(-30, 0)))
		{
			UIKeyPressForm::Show();
			m_EditShortcutValue = node;
		}ImGui::SameLine();

		if (ImGui::Button("X", ImVec2(-1, 0)))
		{
			xr_shortcut val;
			if (V->ApplyValue(val))Modified();
		}
	}
	break;
	case PROP_BOOLEAN:
	{
		BOOLValue* V = dynamic_cast<BOOLValue*>(node->GetFrontValue()); VERIFY(V);
		BOOL new_val_as_BOOL = V->GetValue();
		node->BeforeEdit<BOOLValue, BOOL>(new_val_as_BOOL);
		bool new_val = new_val_as_BOOL;
		if (ImGui::Checkbox("##value", &new_val))
		{
			new_val_as_BOOL = new_val;
			if (node->AfterEdit<BOOLValue, BOOL>(new_val_as_BOOL))
				if (node->ApplyValue<BOOLValue, BOOL>(new_val_as_BOOL))
				{
					Modified();
				}
		}

	}
	break;
	case PROP_FLAG:
	{
		bool change = false;
		if (!FlagOnEdit<u8>(node, change))
			if (!FlagOnEdit<u16>(node, change))
				if (!FlagOnEdit<u32>(node, change))
					R_ASSERT(false);
		if (change)Modified();
	}
	break;
	case PROP_VECTOR:
	{
		VectorValue* V = dynamic_cast<VectorValue*>(node->GetFrontValue()); R_ASSERT(V);
		Fvector edit_val = V->GetValue();

		node->BeforeEdit<VectorValue, Fvector>(edit_val);
		float vector[3] = { edit_val.x,edit_val.y,edit_val.z };
		if (ImGui::InputFloat3("##value", vector, V->dec))
		{
			for (int i = 0; i < 3; i++)
			{
				if (V->lim_mn[i] > vector[i])
				{
					vector[i] = V->lim_mn[i];
				}
				else if (V->lim_mx[i] < vector[i])
				{
					vector[i] = V->lim_mx[i];
				}
				edit_val[i] = vector[i];
			}
			if (node->AfterEdit<VectorValue, Fvector>(edit_val))
				if (node->ApplyValue<VectorValue, Fvector>(edit_val))
				{
					Modified();
				}
		}

	}
	break;
	case PROP_COLOR:
	{
		U32Value* V = dynamic_cast<U32Value*>(node->GetFrontValue()); R_ASSERT(V);
		u32 edit_val = V->GetValue();

		node->BeforeEdit<U32Value, u32>(edit_val);
		u32 a = color_get_A(edit_val);
		float color[3] = { color_get_R(edit_val) / 255.f, color_get_G(edit_val) / 255.f, color_get_B(edit_val) / 255.f };
		if (ImGui::ColorEdit3("##value", color))
		{
			edit_val = color_rgba_f(color[0], color[1], color[2], 1.f);
			edit_val = subst_alpha(edit_val, a);
			if (node->AfterEdit<U32Value, u32>(edit_val))
				if (node->ApplyValue<U32Value, u32>(edit_val)) {
					Modified();
				}
		}
	}
	break;

	case PROP_FCOLOR:
	{
		ColorValue* V = dynamic_cast<ColorValue*>(node->GetFrontValue()); R_ASSERT(V);
		Fcolor edit_val = V->GetValue();

		node->BeforeEdit<ColorValue, Fcolor>(edit_val);
		float a = edit_val.a;
		float color[3] = { edit_val.r,edit_val.b,edit_val.b };
		if (ImGui::ColorEdit3("##value", color))
		{
			edit_val.r = color[0];
			edit_val.g = color[1];
			edit_val.b = color[2];
			edit_val.a = a;
			if (node->AfterEdit<ColorValue, Fcolor>(edit_val))
				if (node->ApplyValue<ColorValue, Fcolor>(edit_val)) {
					Modified();
				}
		}
	}
	break;
	case PROP_VCOLOR:
	{
		VectorValue* V = dynamic_cast<VectorValue*>(node->GetFrontValue()); R_ASSERT(V);
		Fvector edit_val = V->GetValue();

		node->BeforeEdit<VectorValue, Fvector>(edit_val);
		float color[3] = { edit_val[0],edit_val[0],edit_val[0] };
		if (ImGui::ColorEdit3("##value", color))
		{
			edit_val[0] = color[0];
			edit_val[0] = color[1];
			edit_val[0] = color[2];
			if (node->AfterEdit<VectorValue, Fvector>(edit_val))
				if (node->ApplyValue<VectorValue, Fvector>(edit_val))
				{
					Modified();
				}
		}
	}
	break;
	case PROP_CHOOSE:
	{

		xr_string text = node->GetDrawText().c_str();
		if (!text[0])text = "<none>";
		if (ImGui::Button(text.c_str()))
		{
			PropItem* prop = node;

			ChooseValue* V = dynamic_cast<ChooseValue*>(prop->GetFrontValue()); VERIFY(V);
			shared_str	edit_val = V->GetValue();
			if (!edit_val.size()) 	edit_val = V->m_StartPath;
			prop->BeforeEdit<ChooseValue, shared_str>(edit_val);
			//
			ChooseItemVec			items;
			if (!V->OnChooseFillEvent.empty())
			{
				V->m_Items = &items;
				V->OnChooseFillEvent(V);
			}
			UIChooseForm::SelectItem(V->m_ChooseID, V->subitem, edit_val.c_str(), 0, V->m_FillParam, 0, items.size() ? &items : 0, V->m_ChooseFlags);
			m_EditChooseValue = prop;

		}

	}
	break;
	case PROP_TOKEN:
	{
		bool change = false;
		if (!TokenOnEdit<u8>(node, change))
			if (!TokenOnEdit<u16>(node, change))
				if (!TokenOnEdit<u32>(node, change))
					R_ASSERT(false);
		if (change)Modified();

	}
	break;

	case PROP_RTOKEN:
	{
		bool change = false;
		if (!RTokenOnEdit<u8>(node, change))
			if (!RTokenOnEdit<u16>(node, change))
				if (!RTokenOnEdit<u32>(node, change))
					R_ASSERT(false);
		if (change)Modified();
	}
	break;
	case PROP_SH_TOKEN:
	{
		TokenValueSH* V = dynamic_cast<TokenValueSH*>(node->GetFrontValue()); R_ASSERT(V);
		u32 edit_value = V->GetValue();
		node->BeforeEdit<TokenValueSH, u32>(edit_value);
		int index = edit_value;
		const char* InTokens[256];
		int i = 0;
		for (; i < V->cnt; i++)
		{
			InTokens[i] = V->items[i].str;
		}
		if (ImGui::Combo("##value", &index, InTokens, i))
		{
			u32 new_val = index;
			if (node->AfterEdit<TokenValueSH, u32>(new_val))
				if (node->ApplyValue<TokenValueSH, u32>(new_val))Modified();
		}
	}
	break;
	case PROP_TEXTURE2:
	{
		CTextValue* T = dynamic_cast<CTextValue*>(node->GetFrontValue()); R_ASSERT(T);
		xr_string edit_val = T->GetValue();
		node->BeforeEdit<CTextValue, xr_string>(edit_val);
		int index = 0; int cnt = 3;
		LPCSTR List[TSTRING_COUNT+1] = { TEXTUREString[0],TEXTUREString[1],TEXTUREString[2],0 };
		if(edit_val == List[1])index = 1;
		else if (edit_val == List[2])index = 2;
		else if (!edit_val.empty()) { List[3] = edit_val.c_str(); index = 3; cnt = 4; }
		else index = 1;
		if (ImGui::Combo("##value", &index, List, cnt))
		{
			if (index == 0)
			{
				UIChooseForm::SelectItem(smTexture, 8, edit_val.c_str(),0,0,0,0,0);
				m_EditTextureValue = node;
			}
			else
			{
				edit_val = List[index];
				if (node->AfterEdit<CTextValue, xr_string>(edit_val)) 
				{
					if (node->ApplyValue<CTextValue, LPCSTR>(edit_val.c_str()))
					{
						Modified();
					}
				}
			}
		}
	}
	break;
	case PROP_CLIST:
	{
		CListValue* V = dynamic_cast<CListValue*>(node->GetFrontValue()); R_ASSERT(V);
		LPCSTR edit_value = V->value;
		int index = 0;
		const char* InTokens[256];
		int i = 0;
		for (; i < V->item_count; i++)
		{
			if (V->items[i] == edit_value)
			{
				index = i;
			}
			InTokens[i] = V->items[i].c_str();
		}
		if (ImGui::Combo("##value", &index, InTokens, i))
		{
			if (node->AfterEdit<CListValue, xr_string>(V->items[index]))
				if (node->ApplyValue<CListValue, LPCSTR>(V->items[index].c_str()))Modified();
		}
	}
	break;
	case PROP_RLIST:
	{
		RListValue* V = dynamic_cast<RListValue*>(node->GetFrontValue()); R_ASSERT(V);
		LPCSTR edit_value = V->value? V->value->c_str():0;
		int index = 0;
		const char* InTokens[256];
		int i = 0;
		for (; i < V->item_count; i++)
		{
			if (V->items[i] == edit_value)
			{
				index = i;
			}
			InTokens[i] = V->items[i].c_str();
		}
		if (ImGui::Combo("##value", &index, InTokens, i))
		{
			if (node->AfterEdit<RListValue, shared_str>(V->items[index]))
				if (node->ApplyValue<RListValue, shared_str>(V->items[index]))Modified();
		}
	}
	break;
	
		case PROP_CTEXT:
		{
			CTextValue* V = dynamic_cast<CTextValue*>(node->GetFrontValue()); R_ASSERT(V);
			node->GetDrawText();
			ImGui::Text(node->GetDrawText().c_str());
			if (ImGui::OpenPopupOnItemClick("EditText", 0))
			{
				if (m_EditTextValueData)xr_delete(m_EditTextValueData);
				m_EditTextValueData = xr_strdup(V->GetValue());
				m_EditTextValueDataSize = xr_strlen(m_EditTextValueData)+1;
				m_EditTextValue = node;
			}
			DrawEditText();
		}
			break;
		case PROP_RTEXT:
		{
			RTextValue* V = dynamic_cast<RTextValue*>(node->GetFrontValue()); R_ASSERT(V);
			node->GetDrawText();
			ImGui::Text(node->GetDrawText().c_str());
			if (ImGui::OpenPopupOnItemClick("EditText", 0))
			{
				if (m_EditTextValueData)xr_delete(m_EditTextValueData);
				m_EditTextValueData = xr_strdup(V->GetValue().c_str()? V->GetValue().c_str():"");
				m_EditTextValueDataSize = xr_strlen(m_EditTextValueData) + 1;
				m_EditTextValue = node;
			}
			DrawEditText();
		}
			break;
		
		case PROP_STEXT:
		{
			STextValue* V = dynamic_cast<STextValue*>(node->GetFrontValue()); R_ASSERT(V);
			node->GetDrawText();
			ImGui::Text(node->GetDrawText().c_str());
			if (ImGui::OpenPopupOnItemClick("EditText", 0))
			{
				if (m_EditTextValueData)xr_delete(m_EditTextValueData);
				m_EditTextValueData = xr_strdup(V->GetValue().c_str());
				m_EditTextValueDataSize = xr_strlen(m_EditTextValueData) + 1;
				m_EditTextValue = node;
			}
			DrawEditText();
		}
			break;
			/*case PROP_TIME:
			break;
		
		
		case PROP_GAMETYPE:
			break;*/
	default:
		//not_implemented();
		ImGui::Text("");
		break;
	}
}