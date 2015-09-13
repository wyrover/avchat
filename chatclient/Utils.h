#pragma once
namespace avc
{
	class Utils
	{
	public:
		Utils();
		~Utils();
		static int XmlToImageList(const std::wstring& xml, std::vector<std::wstring>* fileList);
		static int XmlTranslateMessage(const std::wstring& xmlMessge,
			const std::map<std::wstring, std::wstring>& fileUrlMap,
			std::wstring* message);
		static int XmlMessageToQtMessage(const std::wstring& xmlMessge, std::vector<std::wstring>* imageList,
			std::wstring* message);
	};
}
