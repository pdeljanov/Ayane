#include "sampleformatconverters.h"

namespace Ayane
{

	const SampleFormatConverters::ConvertFunction SampleFormatConverters::Converter[] =
	{
		&SampleFormatConverters::ConvertFunctionName ( SampleInt16 ),
		&SampleFormatConverters::ConvertFunctionName ( SampleInt24 ),
		&SampleFormatConverters::ConvertFunctionName ( SampleInt32 )
	};
	
	
}