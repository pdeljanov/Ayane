#pragma once

namespace Stargazer
{
    namespace Audio
    {
        
        class Duration
        {
        public:
            
            Duration ( double seconds );
            Duration ( unsigned int minutes, double seconds );
            Duration ( unsigned int hours, unsigned int minutes, double seconds );
            Duration ( unsigned int days, unsigned int hours, unsigned int minutes, double seconds );
            
            double totalDays() const;
            double totalHours() const;
            double totalMinutes() const;
            double totalSeconds() const;
            
            unsigned int days() const;
            unsigned int hours() const;
            unsigned int minutes() const;
            double seconds() const;
            
            Duration& operator= ( const Duration& rhs );
            
            bool operator== ( const Duration &rhs ) const;
            bool operator!= ( const Duration &rhs ) const;
            bool operator> ( const Duration &rhs ) const;
            bool operator>= ( const Duration &rhs ) const;
            bool operator< ( const Duration &rhs ) const;
            bool operator<= ( const Duration &rhs ) const;
            
            Duration operator+ ( const Duration &rhs );
            Duration operator- ( const Duration &rhs );
            
        private:
            
            double m_time;
            
        };
        
    }
    
}
