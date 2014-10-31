#ifndef __COLOR__
#define __COLOR__

class Colors
{
public:
	static float White[4];
	static float Black[4];
	static float Red[4];
	static float Green[4];
	static float Blue[4];
	static float Yellow[4];
	static float Cyan[4];
	static float Magenta[4];
	static float Silver[4];
	static float LightSteelBlue[4];
};

float Colors::White[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
float Colors::Black[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
float Colors::Red[4] = { 1.0f, 0.0f, 0.0f, 1.0f };
float Colors::Green[4] = { 0.0f, 1.0f, 0.0f, 1.0f };
float Colors::Blue[4] = { 0.0f, 0.0f, 1.0f, 1.0f };
float Colors::Yellow[4] = { 1.0f, 1.0f, 0.0f, 1.0f };
float Colors::Cyan[4] = { 0.0f, 1.0f, 1.0f, 1.0f };
float Colors::Magenta[4] = { 1.0f, 0.0f, 1.0f, 1.0f };
float Colors::Silver[4] = { 0.75f, 0.75f, 0.75f, 1.0f };
float Colors::LightSteelBlue[4] = { 0.69f, 0.77f, 0.87f, 1.0f };

#endif // !__COLOR__



