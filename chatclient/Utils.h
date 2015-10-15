#pragma once

#include <string>
#include <map>
#include <vector>

namespace avc
{
	class Utils
	{
	public:
		Utils();
		~Utils();
		static int XmlToImageList(const std::u16string& xml, std::vector<std::u16string>* fileList);
		static int XmlTranslateMessage(const std::u16string& xmlMessge,
			const std::map<std::u16string, std::u16string>& fileUrlMap,
			std::u16string* message);
		static int XmlMessageToQtMessage(const std::u16string& xmlMessge, std::vector<std::u16string>* imageList,
			std::u16string* message);
	};
}
