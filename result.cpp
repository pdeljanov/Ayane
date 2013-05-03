#include "result.h"

namespace Ayane
{

  Result::Result() : 
    m_errorCode( NoError )
  {

  }

  Result::Result ( ErrorCode errorCode ) : 
    m_errorCode( errorCode )
  {
    
  }
  
  Result::Result ( ErrorCode errorCode, const std::string& errorMessage ) : 
    m_errorMessage( errorMessage ), m_errorCode( errorCode )
  {

  }

  ErrorCode Result::errorCode() const
  {
    return m_errorCode;
  }
  
  const std::string& Result::errorMessage() const
  {
    return m_errorMessage;
  }
  
  bool Result::isError() const
  {
    return (m_errorCode != NoError);
  }


}
