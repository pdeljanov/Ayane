#pragma once

#include <string>

namespace Ayane
{

  typedef int ErrorCode;
  
  enum
  { 
    NoError          = 0,
    AllocationFailed = 1,
    CustomError      = 2
  };
  
  class Result
  {
    public:

      Result();
      Result( ErrorCode errorCode );
      Result( ErrorCode errorCode, const std::string &errorMessage );
      
      const std::string &errorMessage() const;
      
      ErrorCode errorCode() const;
      
      bool isError() const;
      
    private:

      std::string m_errorMessage;
      ErrorCode m_errorCode;
      
  };

}
