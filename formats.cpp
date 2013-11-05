/*  Ayane
 *
 *  Created by Philip Deljanov on 11-10-07.
 *  Copyright 2011 Philip Deljanov. All rights reserved.
 *
 */
#include "formats.h"

using namespace Stargazer::Audio;

// Define storage for descriptor and converter tables.
constexpr SampleFormats::Descriptor SampleFormats::descriptorTable[];

/* --- SampleFormats::convertMany(...) Instantiations --- */

/*
template<> void SampleFormats::convertMany(SampleUInt8*, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleInt16*, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleInt16*, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleInt16*, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleInt16*, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleInt16*, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleInt32*, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleInt32*, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleInt32*, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleInt32*, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleInt32*, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleFloat32*, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleFloat64*, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleUInt8*, int, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, int, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, int, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, int, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleUInt8*, int, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleInt16*, int, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleInt16*, int, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleInt16*, int, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleInt16*, int, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleInt16*, int, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleInt32*, int, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleInt32*, int, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleInt32*, int, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleInt32*, int, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleInt32*, int, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleFloat32*, int, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, int, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, int, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, int, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleFloat32*, int, SampleFloat64*, int);

template<> void SampleFormats::convertMany(SampleFloat64*, int, SampleUInt8*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, int, SampleInt16*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, int, SampleInt32*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, int, SampleFloat32*, int);
template<> void SampleFormats::convertMany(SampleFloat64*, int, SampleFloat64*, int);

*/