#pragma once

///////////////////////////////////////////////////////////////////////
//서버에서 쓰기에는 않좋은 타이머... 소멸자 부분을 수정할 필요가 있음
///////////////////////////////////////////////////////////////////////
class SFTimer
{
public:
  SFTimer() { 
    // register timer by adding it to static vector
    m_Timers.push_back(this);
    Stop(); 

#ifdef _DEBUG
    char buf[256]; 
    _snprintf(buf, sizeof(buf), 
      "Timer created! Total Timers: %d\n", m_Timers.size());
    OutputDebugStringA(buf);
#endif
  }
  virtual ~SFTimer() { 
    // delete timer from vector
    for (std::vector<SFTimer *>::iterator i = m_Timers.begin();
         i != m_Timers.end(); i++) {
      if ((*i) == this) {
        m_Timers.erase(i);

#ifdef _DEBUG
        char buf[256]; 
        _snprintf(buf, sizeof(buf), 
          "Timer deleted! Total Timers: %d\n", m_Timers.size());
        OutputDebugStringA(buf);
#endif
        break;
      }
    }
  }
  
  void Start() { m_bIsRunning = true;  }
  void Pause() { m_bIsRunning = false; }
  void Stop()  { Pause(); m_fTime = 0; }
  void Begin() { Stop(); Start(); }
  void BeginWithDelay(float fDelay) { m_fTime = -fDelay; Start(); }
    
  void SetTime(float t) { m_fTime = t; }
  float GetTime()       { return(m_fTime); }

  bool IsRunning() { return(m_bIsRunning); }
  
  void Update(float fElapsedTime) {
    if (m_bIsRunning) {
      m_fTime += fElapsedTime;
    }
  }

  // these are here so you can call 1 function to update all timers
  static std::vector<SFTimer *> m_Timers;
  static void UpdateAll(float fElapsedTime) {
    for (std::vector<SFTimer *>::iterator i = m_Timers.begin();
         i != m_Timers.end(); i++) {
      (*i)->Update(fElapsedTime);
    }
  }

protected:
  bool m_bIsRunning;
  float m_fTime;
};