﻿/*
 * (c) Copyright Ascensio System SIA 2010-2018
 *
 * This program is a free software product. You can redistribute it and/or
 * modify it under the terms of the GNU Affero General Public License (AGPL)
 * version 3 as published by the Free Software Foundation. In accordance with
 * Section 7(a) of the GNU AGPL its Section 15 shall be amended to the effect
 * that Ascensio System SIA expressly excludes the warranty of non-infringement
 * of any third-party rights.
 *
 * This program is distributed WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR  PURPOSE. For
 * details, see the GNU AGPL at: http://www.gnu.org/licenses/agpl-3.0.html
 *
 * You can contact Ascensio System SIA at Lubanas st. 125a-25, Riga, Latvia,
 * EU, LV-1021.
 *
 * The  interactive user interfaces in modified source and object code versions
 * of the Program must display Appropriate Legal Notices, as required under
 * Section 5 of the GNU AGPL version 3.
 *
 * Pursuant to Section 7(b) of the License you must retain the original Product
 * logo when distributing the program. Pursuant to Section 7(e) we decline to
 * grant you any rights under trademark law for use of our trademarks.
 *
 * All the Product's GUI elements, including illustrations and icon sets, as
 * well as technical writing content are licensed under the terms of the
 * Creative Commons Attribution-ShareAlike 4.0 International. See the License
 * terms at http://creativecommons.org/licenses/by-sa/4.0/legalcode
 *
 */
#include "Readers.h"


namespace BinDocxRW {

void InnerColorToOOX(rPr& oRPr, ComplexTypes::Word::CColor& oColor)
{
	if (oRPr.bColor)
	{
		oColor.m_oVal.Init();
		oColor.m_oVal->SetValue(SimpleTypes::hexcolorRGB);
		oColor.m_oVal->Set_R(oRPr.Color.R);
		oColor.m_oVal->Set_G(oRPr.Color.G);
		oColor.m_oVal->Set_B(oRPr.Color.B);
	}
	if (oRPr.bThemeColor && oRPr.ThemeColor.IsNoEmpty())
	{
		if(oRPr.ThemeColor.Auto && !oRPr.bColor)
		{
			oColor.m_oVal.Init();
			oColor.m_oVal->SetValue(SimpleTypes::hexcolorAuto);
		}
		if(oRPr.ThemeColor.bColor)
		{
			oColor.m_oThemeColor.Init();
			oColor.m_oThemeColor->SetValue((SimpleTypes::EThemeColor)oRPr.ThemeColor.Color);
		}
		if(oRPr.ThemeColor.bTint)
		{
			oColor.m_oThemeTint.Init();
			oColor.m_oThemeTint->SetValue(oRPr.ThemeColor.Tint);
		}
		if(oRPr.ThemeColor.bShade)
		{
			oColor.m_oThemeShade.Init();
			oColor.m_oThemeShade->SetValue(oRPr.ThemeColor.Shade);
		}
	}
}

int Binary_VbaProjectTableReader::Read()
{
	m_oFileWriter.m_pVbaProject = new OOX::VbaProject(NULL);
    m_oFileWriter.m_pVbaProject->fromPPTY(&m_oBufferedStream);

    return c_oSerConstants::ReadOk;
}
//-------------------------------------------------------------------------------------	
Binary_HdrFtrTableReader::Binary_HdrFtrTableReader(NSBinPptxRW::CBinaryFileReader& poBufferedStream, Writers::FileWriter& oFileWriter, CComments* pComments):Binary_CommonReader(poBufferedStream),m_oFileWriter(oFileWriter),m_oHeaderFooterWriter(oFileWriter.m_oHeaderFooterWriter),m_pComments(pComments)
{
}
int Binary_HdrFtrTableReader::Read()
{
	int res = c_oSerConstants::ReadOk;
	READ_TABLE_DEF(res, this->ReadHdrFtrContent, NULL);
	return res;
}
int Binary_HdrFtrTableReader::ReadHdrFtrContent(BYTE type, long length, void* poResult)
{
	int res = c_oSerConstants::ReadOk;
	if ( c_oSerHdrFtrTypes::Header == type || c_oSerHdrFtrTypes::Footer == type )
	{
		nCurType = type;
		READ1_DEF(length, res, this->ReadHdrFtrFEO, poResult);
	}
	else
		res = c_oSerConstants::ReadUnknown;
	return res;
}
int Binary_HdrFtrTableReader::ReadHdrFtrFEO(BYTE type, long length, void* poResult)
{
	int res = c_oSerConstants::ReadOk;
	if ( c_oSerHdrFtrTypes::HdrFtr_First == type || c_oSerHdrFtrTypes::HdrFtr_Even == type || c_oSerHdrFtrTypes::HdrFtr_Odd == type )
	{
		nCurHeaderType = type;
		READ1_DEF(length, res, this->ReadHdrFtrItem, poResult);
	}
	else
		res = c_oSerConstants::ReadUnknown;
	return res;
}
int Binary_HdrFtrTableReader::ReadHdrFtrItem(BYTE type, long length, void* poResult)
{
	int res = c_oSerConstants::ReadOk;
	if ( c_oSerHdrFtrTypes::HdrFtr_Content == type )
	{
		Writers::HdrFtrItem* poHdrFtrItem = NULL;
		switch(nCurHeaderType)
		{
		case c_oSerHdrFtrTypes::HdrFtr_First:poHdrFtrItem = new Writers::HdrFtrItem(SimpleTypes::hdrftrFirst);break;
		case c_oSerHdrFtrTypes::HdrFtr_Even:poHdrFtrItem = new Writers::HdrFtrItem(SimpleTypes::hdrftrEven);break;
		case c_oSerHdrFtrTypes::HdrFtr_Odd:poHdrFtrItem = new Writers::HdrFtrItem(SimpleTypes::hdrftrDefault);break;
		}
		if(NULL != poHdrFtrItem)
		{
			if(nCurType == c_oSerHdrFtrTypes::Header)
			{
				m_oHeaderFooterWriter.m_aHeaders.push_back(poHdrFtrItem);
                poHdrFtrItem->m_sFilename = L"header" + std::to_wstring((int)m_oHeaderFooterWriter.m_aHeaders.size()) + L".xml";
			}
			else
			{
				m_oHeaderFooterWriter.m_aFooters.push_back(poHdrFtrItem);
                poHdrFtrItem->m_sFilename = L"footer" + std::to_wstring((int)m_oHeaderFooterWriter.m_aFooters.size()) + L".xml";
			}
			m_oFileWriter.m_pDrawingConverter->SetDstContentRels();
			Binary_DocumentTableReader oBinary_DocumentTableReader(m_oBufferedStream, m_oFileWriter, poHdrFtrItem->Header, m_pComments);
			READ1_DEF(length, res, this->ReadHdrFtrItemContent, &oBinary_DocumentTableReader);

            OOX::CPath fileRelsPath = m_oFileWriter.m_oDocumentWriter.m_sDir +	FILE_SEPARATOR_STR + L"word" + 
																				FILE_SEPARATOR_STR + L"_rels"+ 
																				FILE_SEPARATOR_STR + poHdrFtrItem->m_sFilename + L".rels";

            m_oFileWriter.m_pDrawingConverter->SaveDstContentRels(fileRelsPath.GetPath());
		}
	}
	else
		res = c_oSerConstants::ReadUnknown;
	return res;
}
int Binary_HdrFtrTableReader::ReadHdrFtrItemContent(BYTE type, long length, void* poResult)
{
	Binary_DocumentTableReader* pBinary_DocumentTableReader = static_cast<Binary_DocumentTableReader*>(poResult);
	return pBinary_DocumentTableReader->ReadDocumentContent(type, length, NULL);
}

}
