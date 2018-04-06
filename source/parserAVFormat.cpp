/*  This file is part of YUView - The YUV player with advanced analytics toolset
*   <https://github.com/IENT/YUView>
*   Copyright (C) 2015  Institut f�r Nachrichtentechnik, RWTH Aachen University, GERMANY
*
*   This program is free software; you can redistribute it and/or modify
*   it under the terms of the GNU General Public License as published by
*   the Free Software Foundation; either version 3 of the License, or
*   (at your option) any later version.
*
*   In addition, as a special exception, the copyright holders give
*   permission to link the code of portions of this program with the
*   OpenSSL library under certain conditions as described in each
*   individual source file, and distribute linked combinations including
*   the two.
*   
*   You must obey the GNU General Public License in all respects for all
*   of the code used other than OpenSSL. If you modify file(s) with this
*   exception, you may extend this exception to your version of the
*   file(s), but you are not obligated to do so. If you do not wish to do
*   so, delete this exception statement from your version. If you delete
*   this exception statement from all source files in the program, then
*   also delete it here.
*
*   This program is distributed in the hope that it will be useful,
*   but WITHOUT ANY WARRANTY; without even the implied warranty of
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
*   GNU General Public License for more details.
*
*   You should have received a copy of the GNU General Public License
*   along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "parserAVFormat.h"

#include "parserAnnexBAVC.h"
#include "parserAnnexBHEVC.h"

/* Some macros that we use to read syntax elements from the bitstream.
* The advantage of these macros is, that they can directly also create the tree structure for the QAbstractItemModel that is 
* used to show the NAL units and their content. The tree will only be added if the pointer to the given tree itemTree is valid.
*/
#define READBITS(into,numBits) {QString code; into=reader.readBits(numBits, &code); if (itemTree) new TreeItem(#into,into,QString("u(v) -> u(%1)").arg(numBits),code, itemTree);}
#define READBITS64(into,numBits) {QString code; into=reader.readBits64(numBits, &code); if (itemTree) new TreeItem(#into,into,QString("u(v) -> u(%1)").arg(numBits),code, itemTree);}
#define READFLAG(into) {into=(reader.readBits(1)!=0); if (itemTree) new TreeItem(#into,into,QString("u(1)"),(into!=0)?"1":"0",itemTree);}

parserAVFormat::parserAVFormat(AVCodecID codec)
{ 
  codecID = codec; 
  if (codecID == AV_CODEC_ID_H264)
    annexBParser.reset(new parserAnnexBAVC());
  else if (codecID == AV_CODEC_ID_HEVC)
    annexBParser.reset(new parserAnnexBHEVC());
}

void parserAVFormat::parseExtradata(QByteArray &extradata)
{
  if (codecID == AV_CODEC_ID_H264)
    parseExtradata_AVC(extradata);
  else if (codecID == AV_CODEC_ID_HEVC)
    parseExtradata_hevc(extradata);
  else
    parseExtradata_generic(extradata);
}

void parserAVFormat::parseExtradata_generic(QByteArray &extradata)
{
  TreeItem *extradataRoot = nullptr;
  if (!nalUnitModel.rootItem.isNull())
  {
    extradataRoot = new TreeItem("Extradata", nalUnitModel.rootItem.data());

    // Log all bytes in the extradata
    for (int i = 0; i < extradata.length(); i++)
    {
      int val = (unsigned char)extradata.at(i);
      QString code = QString("%1 (0x%2)").arg(val, 8, 2, QChar('0')).arg(val, 2, 16, QChar('0'));
      TreeItem *byteValue = new TreeItem(QString("Byte %1").arg(i), val, "b(8)", code, extradataRoot);
    }
  }
}

void parserAVFormat::parseExtradata_AVC(QByteArray &extradata)
{
  // TODO:
  parseExtradata_generic(extradata);
}

void parserAVFormat::parseExtradata_hevc(QByteArray &extradata)
{
  if (extradata.at(0) != 1)
    throw std::logic_error("Unsupported extradata format (configurationVersion != 1)");

  TreeItem *extradataRoot = new TreeItem("Extradata (HEVC hvcC format)", nalUnitModel.rootItem.data());
  hvcC h;
  h.parse_hvcC(extradata, extradataRoot, annexBParser);
}


void parserAVFormat::parseAVPacketData(int packetID, QByteArray &avpacketData)
{

}

void parserAVFormat::hvcC::parse_hvcC(QByteArray &hvcCData, TreeItem *itemTree, QScopedPointer<parserAnnexB> &annexBParser)
{
  sub_byte_reader reader(hvcCData);
  reader.disableEmulationPrevention();

  int reserved_4onebits, reserved_5onebits, reserver_6onebits;

  // The first 22 bytes are the hvcC header
  READBITS(configurationVersion, 8);
  if (configurationVersion != 1)
    throw std::logic_error("Only configuration version 1 supported.");
  READBITS(general_profile_space, 2);
  READFLAG(general_tier_flag)
  READBITS(general_profile_idc, 5);
  READBITS(general_profile_compatibility_flags, 32);
  READBITS64(general_constraint_indicator_flags, 48);
  READBITS(general_level_idc, 8);
  READBITS(reserved_4onebits, 4);
  if (reserved_4onebits != 15)
    throw std::logic_error("The reserved 4 one bits should all be one.");
  READBITS(min_spatial_segmentation_idc, 12);
  READBITS(reserver_6onebits, 6);
  if (reserver_6onebits != 63)
    throw std::logic_error("The reserved 6 one bits should all be one.");
  READBITS(parallelismType, 2);
  READBITS(reserver_6onebits, 6);
  if (reserver_6onebits != 63)
    throw std::logic_error("The reserved 6 one bits should all be one.");
  READBITS(chromaFormat, 2);
  READBITS(reserved_5onebits, 5);
  if (reserved_5onebits != 31)
    throw std::logic_error("The reserved 6 one bits should all be one.");
  READBITS(bitDepthLumaMinus8, 3);
  READBITS(reserved_5onebits, 5);
  if (reserved_5onebits != 31)
    throw std::logic_error("The reserved 6 one bits should all be one.");
  READBITS(bitDepthChromaMinus8, 3);
  READBITS(avgFrameRate, 16);
  READBITS(constantFrameRate, 2);
  READBITS(numTemporalLayers, 3);
  READFLAG(temporalIdNested);
  READBITS(lengthSizeMinusOne, 2);
  READBITS(numOfArrays, 8);

  // Now parse the contained raw NAL unit arrays
  for (int i = 0; i < numOfArrays; i++)
  {
    hvcC_naluArray a;
    a.parse_hvcC_naluArray(i, reader, itemTree, annexBParser);
    naluArrayList.append(a);
  }
}

void parserAVFormat::hvcC_naluArray::parse_hvcC_naluArray(int arrayID, sub_byte_reader &reader, TreeItem *root, QScopedPointer<parserAnnexB> &annexBParser)
{
  // Create a new TreeItem root for this nalArray
  // The macros will use this variable to add all the parsed variables
  TreeItem *const itemTree = root ? new TreeItem(QString("nal unit array %1").arg(arrayID), root) : nullptr;

  // The next 3 bytes contain info about the array
  READFLAG(array_completeness);
  READFLAG(reserved_flag_false);
  if (reserved_flag_false)
    throw std::logic_error("The reserved_flag_false should be false.");
  READBITS(NAL_unit_type, 6);
  READBITS(numNalus, 16);
  
  for (int i = 0; i < numNalus; i++)
  {
    hvcC_nalUnit nal;
    nal.parse_hvcC_nalUnit(i, reader, itemTree, annexBParser);
    nalList.append(nal);
  }
}

void parserAVFormat::hvcC_nalUnit::parse_hvcC_nalUnit(int unitID, sub_byte_reader &reader, TreeItem *root, QScopedPointer<parserAnnexB> &annexBParser)
{
  // Create a new TreeItem root for this nalUnit
  // The macros will use this variable to add all the parsed variables
  TreeItem *const itemTree = root ? new TreeItem(QString("nal unit %1").arg(unitID), root) : nullptr;

  READBITS(nalUnitLength, 16);

  // Get the bytes of the raw nal unit to pass to the "real" hevc parser
  QByteArray nalData = reader.readBytes(nalUnitLength);

  // Let the hevc annexB parser parse this
  annexBParser->parseAndAddNALUnit(unitID, nalData);
}