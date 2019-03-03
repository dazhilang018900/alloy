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

#ifndef SRC_UI_ALLOYFILEWIDGET_H_
#define SRC_UI_ALLOYFILEWIDGET_H_
#include "ui/AlloyBorderComposite.h"
#include "ui/AlloyAdjustableComposite.h"
#include "ui/AlloyTextWidget.h"
#include "ui/AlloyListBox.h"
#include "ui/AlloyButton.h"
#include "ui/AlloySelectionBox.h"
#include "system/AlloyFileUtil.h"
namespace aly {
class FileDialog;
class FileEntry: public ListEntry {
private:
	std::string creationTime;
	std::string lastModifiedTime;
	std::string lastAccessTime;
	std::string fileSize;
public:
	FileDescription fileDescription;
	FileEntry(FileDialog* dialog, const std::string& name, float fontHeight);
	void setValue(const FileDescription& fileDescription);
};
class FileField: public TextField {
protected:
	std::vector<std::string> segmentedPath;
	std::shared_ptr<SelectionBox> selectionBox;
	std::shared_ptr<TimerTask> showTimer;
	bool autoSuggest;
	bool directoryInput;
	int preferredFieldSize;
	void updateSuggestionBox(AlloyContext* context, bool forceValue);
public:
	void setEnableAutoSugest(bool b) {
		autoSuggest = b;
	}
	void setPreferredFieldSize(int w) {
		preferredFieldSize = w;
	}
	AColor textColor = MakeColor(Theme::Default.LIGHTER);
	virtual bool onEventHandler(AlloyContext* context, const InputEvent& event)
			override;
	virtual inline ~FileField() {
	}
	void hideDropDown(AlloyContext* context) {
		selectionBox->setVisible(false);
		context->removeOnTopRegion(selectionBox.get());
	}
	FileField(const std::string& name, const AUnit2D& position,
			const AUnit2D& dimensions, bool directoryInput = false);
	virtual void draw(AlloyContext* context) override;
	virtual void setValue(const std::string& value) override;
};
struct FileFilterRule {
	std::string name;
	std::vector<std::string> extensions;
	FileFilterRule(const std::string& name, const std::string& extension) :
			name(name) {
		extensions.push_back(extension);
	}
	FileFilterRule(const FileFilterRule& rule) :
			name(rule.name), extensions(rule.extensions) {
	}
	FileFilterRule(const std::string& name = "") :
			name(name) {
	}
	template<class Archive> void save(Archive& ar) const {
		ar(CEREAL_NVP(name), CEREAL_NVP(extensions));
	}
	template<class Archive> void load(Archive& ar) {
		ar(CEREAL_NVP(name), CEREAL_NVP(extensions));
	}
	FileFilterRule(const std::string& name,
			std::initializer_list<std::string> extension) :
			name(name) {
		for (std::string ext : extension) {
			extensions.push_back(ext);
		}
	}
	std::string toString();
	virtual bool accept(const std::string& file);
	virtual ~FileFilterRule() {
	}
};
class FileButton;

class FileDialog: public AdjustableComposite {
private:
	std::vector<std::shared_ptr<FileFilterRule>> filterRules;
	std::shared_ptr<FileField> fileLocation;
	std::shared_ptr<Composite> directoryTree;
	std::shared_ptr<ListBox> directoryList;
	std::shared_ptr<Selection> fileTypeSelect;
	std::shared_ptr<TextField> newFolderField;
	std::shared_ptr<TextIconButton> actionButton;
	std::shared_ptr<IconButton> upDirButton;
	std::shared_ptr<IconButton> makeDirButton;
	std::shared_ptr<IconButton> cancelButton;
	std::shared_ptr<BorderComposite> containerRegion;
	std::string lastDirectory;
	void setSelectedFile(const std::string& file, bool changeDirectory = true);
	const FileDialogType type;
	pixel fileEntryHeight;
	bool valid = false;
	bool dragAccept = false;
	bool glassState=false;
	void updateDirectoryList();
	bool updateValidity();
public:
	virtual bool acceptDragEvent(const pixel2& cursor) const override;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension);
	void addFileExtensionRule(const FileFilterRule& rule);
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string> & extension);
	friend class FileEntry;
	std::function<void(const std::vector<std::string>&)> onSelect;
	virtual void draw(AlloyContext* context) override;
	virtual void setVisible(bool v) override;
	FileDialog(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const FileDialogType& type, pixel fileEntryHeight = 30);
	void update();
	void setValue(const std::string& file);
	std::string getValue() const;
	void setFileExtensionRule(int index) {
		fileTypeSelect->setValue(index);
		updateDirectoryList();
	}
};
class FileSelector: public BorderComposite {
private:
	std::shared_ptr<FileField> fileLocation;
	std::shared_ptr<IconButton> openIcon;
	std::shared_ptr<FileDialog> fileDialog;
	bool directoryInput;
public:
	void setTextColor(const AColor& c);
	std::function<void(const std::string& file)> onChange;
	virtual void appendTo(TabChain& chain) override;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension) ;
	void addFileExtensionRule(const FileFilterRule& rule);
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string>& extension);

	void setFileExtensionRule(int index) ;
	std::string getValue();
	FileSelector(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool directoryInput = false);
	void setValue(const std::string& file);

	void openFileDialog(AlloyContext* context,
			const std::string& workingDirectory = GetCurrentWorkingDirectory());
};
class FileButton: public IconButton {
private:
	std::shared_ptr<FileDialog> fileDialog;
public:

	std::function<void(const std::vector<std::string>& file)> onOpen;
	std::function<void(const std::string& file)> onSave;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension) ;
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string>& extension);
	void addFileExtensionRule(const FileFilterRule& rule);
	void setFileExtensionRule(int index) ;
	FileButton(const std::string& name, const AUnit2D& pos, const AUnit2D& dims,
			const FileDialogType& type);
	void setValue(const std::string& file);
	std::string getValue();
	void openFileDialog(AlloyContext* context,
			const std::string& workingDirectory = GetCurrentWorkingDirectory());
};
class MultiFileEntry: public ListEntry {
private:
	std::string fileName;
public:
	MultiFileEntry(ListBox* listBox, const std::string& name, float fontHeight);
	void setValue(const std::string& file);
	std::string getValue() const {
		return fileName;
	}
};

class MultiFileSelector: public Composite {
protected:
	std::shared_ptr<ListBox> valueRegion;
	std::shared_ptr<FileButton> openFileButton;
	std::shared_ptr<IconButton> upButton;
	std::shared_ptr<IconButton> downButton;
	std::shared_ptr<IconButton> eraseButton;
	bool directoryInput;
	float entryHeight;
	void update();
	void fireEvent();
public:
	std::function<void(const std::vector<std::string>& files)> onChange;
	virtual void appendTo(TabChain& chain) override;
	void addFileExtensionRule(const std::string& name,
			const std::string& extension);
	void addFileExtensionRule(const std::string& name,
			const std::initializer_list<std::string>& extension);
	void addFileExtensionRule(const FileFilterRule& rule);
	void setFileExtensionRule(int index);
	void addFiles(const std::vector<std::string>& files);
	void clearEntries();
	MultiFileSelector(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dims, bool directoryInput = false,
			float entryHeight = 30.0f);
};

typedef std::shared_ptr<FileButton> FileButtonPtr;
typedef std::shared_ptr<FileSelector> FileSelectorPtr;
typedef std::shared_ptr<FileDialog> FileDialogPtr;
typedef std::shared_ptr<FileField> FileFieldPtr;
typedef std::shared_ptr<MultiFileEntry> MultiFileEntryPtr;
typedef std::shared_ptr<MultiFileSelector> MultiFileSelectorPtr;
}
#endif /* SRC_UI_ALLOYFILEWIDGET_H_ */
