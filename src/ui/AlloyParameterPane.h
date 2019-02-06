/*
 * Copyright(C) 2015, Blake C. Lucas, Ph.D. (img.science@gmail.com)
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
#ifndef ALLOYPARAMETERPANE_H_
#define ALLOYPARAMETERPANE_H_
#include "ui/AlloyUI.h"
#include "common/AlloyAny.h"
#include <string>
#include <vector>
namespace aly {
template<int M, int N> class MatrixField: public aly::Composite {
protected:
	aly::matrix<float, M, N> Mat;
	aly::ModifiableNumberPtr matField[M][N];
public:
	std::function<void(MatrixField<M, N>*)> onTextEntered;
	MatrixField(const std::string& name, const aly::AUnit2D& pos,
			const aly::AUnit2D& dims, const aly::matrix<float, M, N>& Mat =
					aly::matrix<float, M, N>()) :
			Composite(name, pos, dims), Mat(Mat) {
		using namespace aly;
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				ModifiableNumberPtr field = ModifiableNumberPtr(
						new ModifiableNumber(
								MakeString() << "M[" << m << "][" << n << "]",
								CoordPercent(n / (float) N, m / (float) M),
								CoordPercent(1.0f / M, 1.0f / N),
								NumberType::Float));
				field->backgroundColor = MakeColor(
						AlloyDefaultContext()->theme.DARK);
				field->borderColor = MakeColor(
						AlloyDefaultContext()->theme.LIGHT);
				field->setNumberValue(Float(Mat[m][n]));
				field->setRoundCorners(true);
				field->setAlignment(HorizontalAlignment::Center,
						VerticalAlignment::Middle);
				field->onTextEntered = [this,m,n](NumberField* field) {
					this->Mat[m][n]=field->getValue().toFloat();
					if(onTextEntered) {
						onTextEntered(this);
					}
				};
				Composite::add(field);
				matField[m][n] = field;
			}
		}
	}
	virtual void appendTo(TabChain& chain) override{
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				chain.add(matField[m][n].get());
			}
		}
	}
	void setValue(const aly::matrix<float, M, N>& m) {
		Mat = m;
		for (int m = 0; m < M; m++) {
			for (int n = 0; n < N; n++) {
				matField[m][n]->setNumberValue(aly::Float(Mat[m][n]));
			}
		}
	}
	aly::matrix<float, M, N> getValue() const {
		return Mat;
	}
};

template<int N> class VectorFloatField: public aly::Composite {
protected:
	aly::vec<float, N> value;
	aly::ModifiableNumberPtr vecField[N];
public:
	std::function<void(VectorFloatField<N>*)> onTextEntered;
	VectorFloatField(const std::string& name, const aly::AUnit2D& pos,
			const aly::AUnit2D& dims,
			const aly::vec<float, N>& Mat = aly::vec<float, N>()) :
			Composite(name, pos, dims), value(Mat) {
		using namespace aly;
		for (int n = 0; n < N; n++) {
			ModifiableNumberPtr field = ModifiableNumberPtr(
					new ModifiableNumber(MakeString() << "V[" << n << "]",
							CoordPercent(n / (float) N, 0.0f),
							CoordPercent(1.0f / N, 1.0f), NumberType::Float));
			field->backgroundColor = MakeColor(
					AlloyDefaultContext()->theme.DARK);
			field->borderColor = MakeColor(AlloyDefaultContext()->theme.LIGHT);
			field->setNumberValue(Float(Mat[n]));
			field->setRoundCorners(true);
			field->setAlignment(HorizontalAlignment::Center,
					VerticalAlignment::Middle);
			field->onTextEntered = [this,n](NumberField* field) {
				this->value[n]=field->getValue().toFloat();
				if(onTextEntered) {
					onTextEntered(this);
				}
			};
			Composite::add(field);
			vecField[n] = field;
		}
	}
	virtual void appendTo(TabChain& chain) override {
		for (int n = 0; n < N; n++) {
			chain.add(vecField[n].get());
		}
	}
	void setValue(const aly::vec<float, N>& m) {
		value = m;
		for (int n = 0; n < N; n++) {
			vecField[n]->setNumberValue(aly::Float(value[n]));
		}
	}
	aly::vec<float, N> getValue() const {
		return value;
	}
};
template<int N> class VectorIntegerField: public aly::Composite {
protected:
	aly::vec<int, N> value;
	aly::ModifiableNumberPtr vecField[N];
public:
	std::function<void(VectorIntegerField<N>*)> onTextEntered;

	VectorIntegerField(const std::string& name, const aly::AUnit2D& pos,
			const aly::AUnit2D& dims,
			const aly::vec<int, N>& Mat = aly::vec<int, N>()) :
			Composite(name, pos, dims), value(Mat) {
		using namespace aly;
		for (int n = 0; n < N; n++) {
			ModifiableNumberPtr field = ModifiableNumberPtr(
					new ModifiableNumber(MakeString() << "V[" << n << "]",
							CoordPercent(n / (float) N, 0.0f),
							CoordPercent(1.0f / N, 1.0f), NumberType::Integer));
			field->backgroundColor = MakeColor(
					AlloyDefaultContext()->theme.DARK);
			field->borderColor = MakeColor(AlloyDefaultContext()->theme.LIGHT);
			field->setRoundCorners(true);
			field->setNumberValue(Integer(Mat[n]));
			field->setAlignment(HorizontalAlignment::Center,
					VerticalAlignment::Middle);
			field->onTextEntered = [this,n](NumberField* field) {
				this->value[n]=field->getValue().toInteger();
				if(onTextEntered) {
					onTextEntered(this);
				}
			};
			Composite::add(field);
			vecField[n] = field;
		}
	}
	virtual void appendTo(TabChain& chain) override {
		for (int n = 0; n < N; n++) {
			chain.add(vecField[n].get());
		}
	}
	void setValue(const aly::vec<int, N>& m) {
		value = m;
		for (int n = 0; n < N; n++) {
			vecField[n]->setNumberValue(aly::Integer(value[n]));
		}
	}
	aly::vec<int, N> getValue() const {
		return value;
	}
};
class ParameterPane: public Composite {
protected:
	static const float SPACING;
	float entryHeight;
	std::vector<std::shared_ptr<AnyInterface>> values;
	void setCommonParameters(const CompositePtr& compRegion,
			const TextLabelPtr& textLabel, const RegionPtr& regionLabel);
	std::list<RegionPtr> groupQueue;
	CompositePtr lastRegion;
	ExpandBarPtr expandBar;
	float estimatedHeight = 0;
	bool lastExpanded = false;
	NumberFieldPtr addNumberFieldItem(const std::string& label, Number& value,
			float aspect = 3.0f);
	void updateGroups();
	template<int M, int N> std::shared_ptr<MatrixField<M, N>> addMatrixFieldInternal(
			const std::string& label, aly::matrix<float, M, N>& value,
			float aspect );
	template<int N> std::shared_ptr<VectorFloatField<N>> addVectorFloatFieldInternal(
			const std::string& label, aly::vec<float, N>& value, float aspect );
	template<int N> std::shared_ptr<VectorIntegerField<N>> addVectorIntegerFieldInternal(
			const std::string& label, aly::vec<int, N>& value, float aspect );
public:
	std::function<void(const std::string& name, const AnyInterface& value)> onChange;
	AColor entryTextColor;
	AColor entryBackgroundColor;
	AColor entryBorderColor;
	AUnit1D entryBorderWidth;
	virtual void clear() override;
	virtual void pack(const pixel2& pos, const pixel2& dims,
			const double2& dpmm, double pixelRatio, bool clamp) override;
	ParameterPane(const std::string& name, const AUnit2D& pos,
			const AUnit2D& dim, float entryHeight = 30.0f);
	CompositePtr addGroup(const std::string& name, bool expanded);
	NumberFieldPtr addNumberField(const std::string& label, Number& value,
			float aspect = 3.0f);
	std::pair<NumberFieldPtr, NumberFieldPtr> addRangeField(
			const std::string& label, Number& lowerValue, Number& upperValue,
			float aspect = 6.0f);
	std::pair<ModifiableNumberPtr, ModifiableNumberPtr> addRangeField(
			const std::string& label, Number& lowerValue, Number& upperValue,
			const Number& minValue, const Number& maxValue,
			float aspect = 9.0f);
	TextFieldPtr addTextField(const std::string& label, std::string& value,
			float aspect = 3.0f);
	TextFieldPtr addRangeField(const std::string& label,
			std::vector<int>& value, const std::string& initialRange = "",
			float aspect = 4.0f);
	ModifiableNumberPtr addNumberField(const std::string& label, Number& value,
			const Number& minValue, const Number& maxValue,
			float aspect = 7.0f);
	SelectionPtr addSelectionField(const std::string& label, int& selectedIndex,
			const std::vector<std::string>& options, float aspect = 4.0f);
	ToggleBoxPtr addToggleBox(const std::string& label, bool& value,
			float aspect = 2.1f);
	CheckBoxPtr addCheckBox(const std::string& label, bool& value,
			float aspect = 1.0f);
	ColorSelectorPtr addColorField(const std::string& label, Color& color,
			float aspect = 3.0f);
	FileSelectorPtr addFileField(const std::string& label, std::string& file,
			float aspect = -1.0f);
	FileSelectorPtr addDirectoryField(const std::string& label,
			std::string& file, float aspect = -1.0f);
	MultiFileSelectorPtr addMultiFileSelector(const std::string& label,
			std::vector<std::string>& files, float aspect = -1.0f);
	MultiFileSelectorPtr addMultiDirectorySelector(const std::string& label,
			std::vector<std::string>& files, float aspect = -1.0f);
	std::shared_ptr<Region> addNumberVectorField(const std::string& label,
			std::vector<Number>& value, const NumberType& type, float aspect =
					-1.0f);

	std::shared_ptr<VectorIntegerField<2>> addVectorIntegerField(
			const std::string& label, aly::vec<int, 2>& value, float aspect =
					-1.0f);
	std::shared_ptr<VectorIntegerField<3>> addVectorIntegerField(
			const std::string& label, aly::vec<int, 3>& value, float aspect =
					-1.0f);
	std::shared_ptr<VectorIntegerField<4>> addVectorIntegerField(
			const std::string& label, aly::vec<int, 4>& value, float aspect =
					-1.0f);
	std::shared_ptr<VectorFloatField<2>> addVectorFloatField(
			const std::string& label, aly::vec<float, 2>& value, float aspect =
					-1.0f);
	std::shared_ptr<VectorFloatField<3>> addVectorFloatField(
			const std::string& label, aly::vec<float, 3>& value, float aspect =
					-1.0f);
	std::shared_ptr<VectorFloatField<4>> addVectorFloatField(
			const std::string& label, aly::vec<float, 4>& value, float aspect =
					-1.0f);
	std::shared_ptr<MatrixField<2, 2>> addMatrixField(const std::string& label,
			aly::matrix<float, 2, 2>& value, float aspect = -1.0f);
	std::shared_ptr<MatrixField<3, 3>> addMatrixField(const std::string& label,
			aly::matrix<float, 3, 3>& value, float aspect = -1.0f);
	std::shared_ptr<MatrixField<3, 4>> addMatrixField(const std::string& label,
			aly::matrix<float, 3, 4>& value, float aspect = -1.0f);
	std::shared_ptr<MatrixField<4, 3>> addMatrixField(const std::string& label,
			aly::matrix<float, 4, 3>& value, float aspect = -1.0f);
	std::shared_ptr<MatrixField<4, 4>> addMatrixField(const std::string& label,
			aly::matrix<float, 4, 4>& value, float aspect = -1.0f);
};

typedef std::shared_ptr<ParameterPane> ParameterPanePtr;
}

#endif
