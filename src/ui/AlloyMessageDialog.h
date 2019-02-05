/*
 * Copyright(C) 2018, Blake C. Lucas, Ph.D. (img.science@gmail.com)
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */
#ifndef SRC_UI_ALLOYMESSAGEDIALOG_H_
#define SRC_UI_ALLOYMESSAGEDIALOG_H_

#include "AlloyComposite.h"
#include "AlloyTextWidget.h"
#include "AlloyButton.h"
namespace aly {


class MessageDialog: public Composite {
protected:
	bool returnValue = false;
	MessageOption option;
	MessageType type;
	TextButtonPtr actionButton;
	std::shared_ptr<Composite> containerRegion;
	std::shared_ptr<TextLabel> textLabel;
public:
	bool getValue() const {
		return returnValue;
	}
	virtual void setVisible(bool v) override;
	void setMessage(const std::string& message);
        std::string getMessage() const;
	MessageDialog(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool wrap, const MessageOption& option,
			const MessageType& type);
	MessageDialog(const std::string& name, bool wrap,
			const MessageOption& option, const MessageType& type);
	virtual void draw(AlloyContext* context) override;
	std::function<void(MessageDialog* dialog)> onSelect;
};

typedef std::shared_ptr<MessageDialog> MessageDialogPtr;
} /* namespace aly */

#endif /* SRC_UI_ALLOYMESSAGEDIALOG_H_ */
