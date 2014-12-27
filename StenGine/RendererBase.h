#ifndef __RENDERERBASE__
#define __RENDERERBASE__

class Renderer {
public:
	//virtual ~Renderer();
	//static Renderer* Instance() { return _instance; }
	virtual bool Init() = 0;
	virtual void Draw() = 0;
	float GetAspectRatio() { return static_cast<float>(m_clientWidth) / static_cast<float>(m_clientHeight); }
	int GetScreenWidth() { return m_clientWidth; }
	int GetScreenHeight() { return m_clientHeight; }


protected:
	//static Renderer* _instance;

	int m_clientWidth;
	int m_clientHeight;
	bool m_enable4xMsaa;
};


#endif