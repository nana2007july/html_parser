/*
* Copyright 2017 the original author or authors.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
*      http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*/
#ifndef NANA_HTML_ANALYSIS_INCLUDED
#define NANA_HTML_ANALYSIS_INCLUDED

#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cstdlib>
#include <exception>


#include "html_element.hpp"


namespace nana{



using std::string;
using std::vector;
using std::istream;
using std::unique_ptr;




//����--------------------------------------------



const bool forwardMatch(const string& targetStr, const string& searchStr);

const bool backwardMatch(const string& targetStr, const string& searchStr);
const bool wildcardMatch(const char *ptn, const char *str);


/**
<pre>
HTML�^�O�̃^�O��͂�����B��ԓ����̊J�n�^�O��������܂ő������Ă�����@�i�K�w�}�b�`�j�B
�y��͗�i��͎�@�ɂ����HTML�^�O�L�q���Ԉ���Ă���ꍇ�Ɍ��ʂ̊K�w�\��������Ă���j�z
�i���́j&lt;html>&lt;form>&lt;div>&lt;/form>&lt;/div>&lt;/html>
�i���ʁj
�@��&lt;html>�J�n�^�O
�@�@��&lt;form>�J�n�^�O
   �@�@�@��&lt;div> �J�n�I�����������m�[�h
	 �@�@�@��&lt;/form>�I���^�O
	��&lt;/html>�I���^�O
</pre>
@param p_ret [out]���̃I�u�W�F�N�g�Ɍ��ʂ�ǉ�����
@param p_allDocParts [in]�S�Ă�HtmlParts������(HTML�L�q�̏��Ԓʂ�)�ɓ��ꂽ����
*/
void analyzeHtmlNode(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts);


/**
<pre>
HTML�^�O�̃^�O��͂�����B��ԋ߂��I���^�O�ƃ}�b�`���Ă�����@�i�����^�O���ǂ����ŏo�����Ƀ^�O�}�b�`�j�B
�y��͗�i��͎�@�ɂ����HTML�^�O�L�q���Ԉ���Ă���ꍇ�Ɍ��ʂ̊K�w�\��������Ă���j�z
�i���́j&lt;html>&lt;form>&lt;div>&lt;/form>&lt;/div>&lt;/html>
�i���ʁj
�@��html�@�J�n�I�����������m�[�h
 �@�@��form�@�J�n�I�����������m�[�h
   �@�@�@��div�@�J�n�I�����������m�[�h
</pre>
@param p_ret [out]���̃I�u�W�F�N�g�Ɍ��ʂ�ǉ�����
@param p_allDocParts [in]�S�Ă�HtmlParts������(HTML�L�q�̏��Ԓʂ�)�ɓ��ꂽ����
*/
void analyzeHtmlNodeBySameTagMatch(HtmlNode& p_ret, const HtmlDocument::HtmlPartUptrs& p_allDocParts);

/**
@brief HTML����͂��A HtmlDocument ���쐬���ĕԂ�HTML�p�[�T�n���h���B
@see analyzeHtmlNode
*/
class DocumentHtmlSaxParserHandler :public SimpleHtmlSaxParserHandler {
public:
	DocumentHtmlSaxParserHandler(){};
	virtual ~DocumentHtmlSaxParserHandler(){};
	//�p�[�X�������ʂ��擾����B���s�O�����ʎ擾���nullptr���Ԃ�B
	unique_ptr<HtmlDocument> result();
};


//---------------------------------------------
/**
@brief HtmlNode�ɃA�N�Z�X���A�^�O���̌���������A�N�Z�T�̊��N���X
@see HtmlNodeVisitor
*/
class HtmlNodeAccessor: noncopyable{
public:
	virtual ~HtmlNodeAccessor(){};
	/** �^�O�ɃA�N�Z�X�i�`�F�b�N�j���� */
	virtual void access(const HtmlNode& p_node) = 0;
	/** �������B���s�O�ɌĂ΂��B */
	virtual void init() = 0;
};

/**
@brief HtmlNode�����Ԃɂ��ׂĖK�₷��N���X�B�K�₵������ HtmlNodeAccessor ���Ăяo���B
@see HtmlNodeAccessor
*/
class HtmlNodeVisitor: noncopyable{
public:
	virtual ~HtmlNodeVisitor(){};
	//
	void access(const HtmlNode& p_node, HtmlNodeAccessor& p_accessor){
		p_accessor.init();
		_access(p_node, p_accessor);
	};
protected:
	void _access(const HtmlNode& p_node, HtmlNodeAccessor& p_accessor){
		//���[�g�m�[�h�ȊO�̏ꍇ
		if(!(p_node.startTag() == nullptr && p_node.endTag() == nullptr))
			p_accessor.access(p_node);
		for(auto i = p_node.begin(); i != p_node.end(); ++i){
			_access(**i, p_accessor);
		}
	};
};

/**
@brief �����̃A�N�Z�X�N���X�����܂Ƃ߂āA�m�[�h�ɃA�N�Z�X�ł���A�N�Z�T�N���X�B
*/
class CompositeAccessor : public HtmlNodeAccessor{
public:
	///new�����I�u�W�F�N�g��n�����ƁB�p���̊Ǘ��͂��̃N���X�ōs���B
	CompositeAccessor& add(HtmlNodeAccessor* p_accs){
		m_nodeAccessorUptrs.push_back(unique_ptr<HtmlNodeAccessor>(p_accs));
		return *this; 
	};
	//
	virtual void access(const HtmlNode& p_node){
		for(auto i = m_nodeAccessorUptrs.begin(); i != m_nodeAccessorUptrs.end(); ++i){
			(*i)->access(p_node);
		}
	};
	//������
	virtual void init(){
		for(auto i = m_nodeAccessorUptrs.begin(); i != m_nodeAccessorUptrs.end(); ++i) (*i)->init();
	};
	/** ���ʂ��擾���邽�߂̃w���p�[�֐�
	@param T [in] add() �����A�N�Z�T�N���X
	@param p_index [in]�A�N�Z�T���w��B add() �������ԁi0�`�j
	*/
	template<class T>
	T& accessor(const int p_index){ return *dynamic_cast<T*>(m_nodeAccessorUptrs[p_index].get()); };
private:
	vector<unique_ptr<HtmlNodeAccessor>> m_nodeAccessorUptrs;
};

/** 
@brief ���Ă��Ȃ��^�O�ƁA�݂��Ⴂ�ɂȂ��Ă���^�O�̒��o������A�N�Z�T�N���X�B 
*/
class EndTagAccessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~EndTagAccessor(){};
	virtual void access(const HtmlNode& p_node);
	//������
	virtual void init(){
		m_nonClosedResult.reset(new SearchResults);
		m_alternatedResult.reset(new SearchResults);
		m_stockMap.clear();
	};
	///���Ă��Ȃ��^�O�̒��o����
	SearchResultsUptr nonClosedResult();
	///�݂��Ⴂ�ɂȂ��Ă���^�O�̒��o����
	SearchResultsUptr alternatedResult(){ return move(m_alternatedResult); };
private:
	SearchResultsUptr m_nonClosedResult;
	SearchResultsUptr m_alternatedResult;
	std::map<std::string, std::vector<const HtmlNode*>> m_stockMap;
};

/**
@brief HTML5�ŋ֎~�ɂȂ����^�O�𒊏o����A�N�Z�T�N���X�B
*/
class DeprecatedInHtml5Accessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~DeprecatedInHtml5Accessor(){};
	virtual void access(const HtmlNode& p_node){
		if(s_deprecatedTagMap.find(p_node.tagName()) != s_deprecatedTagMap.end()){
			m_result->push_back(&p_node);
		}
	};
	//������
	virtual void init(){ m_result.reset(new SearchResults); };
	//���ʎ擾
	SearchResultsUptr result(){
		return move(m_result);
	};
private:
	SearchResultsUptr m_result;
	static const std::map<std::string, int> s_deprecatedTagMap;
};


/** 
@brief img�^�O��alt���������݂��Ȃ��^�O�𒊏o����A�N�Z�T�N���X�B�i�A�N�Z�V�r���e�B�̊ϓ_�ł͏d�v�ȃ`�F�b�N���ځB�j
*/
class ImgAltAccessor : public HtmlNodeAccessor{
public:
	typedef vector<const HtmlNode*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	virtual ~ImgAltAccessor(){};
	virtual void access(const HtmlNode& p_node){
		if(p_node.tagName() != "img") return;
		if(p_node.startTag() == nullptr) return; 
		if(!p_node.startTag()->hasAttr("alt", 0)){
			//alt���������݂��Ȃ��ꍇ
			m_result->push_back(&p_node);
		}
	};
	//������
	virtual void init(){ m_result.reset(new SearchResults); };
	//
	SearchResultsUptr result(){
		return move(m_result);
	};
private:
	SearchResultsUptr m_result;
};


//-------------------------------
namespace path{

	/**
	@brief XPath���ǂ����������߂̊��N���X�B�p�X���o�̎��s�҂̂P��\���B
	*/
	class HtmlPath: noncopyable{
	public:
		typedef vector<const HtmlNode*> HtmlNodePtrs;
		virtual ~HtmlPath(){};
		/** �����̔z���̃m�[�h���t�B���^���A�w��̃m�[�h�𒊏o����B */
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePtrs) = 0;
	};

	/**
	@brief �^�O���Ƒ����̌����i1�K�w/tag�̎w��j�B
	*/
	class PathHtmlPath :public HtmlPath{
	public:
		/**
		@param p_tagName [in]�^�O�������C���h�J�[�h�Ŏw��
		*/
		PathHtmlPath(const string& p_tagName):m_tagName(p_tagName){};
		virtual ~PathHtmlPath(){};
		/**
		@param p_htmlPath [in]�q��(pred)���w��
		*/
		PathHtmlPath* add(HtmlPath* p_htmlPathPtr){
			m_htmlPathUptrList.push_back(unique_ptr<HtmlPath>(p_htmlPathPtr));
			return this;
		};
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	protected:
		///�t�B���^�������ʂ�retVec�ɒǉ�����
		void match(HtmlNodePtrs& retVec, const vector<const HtmlNode*>& p_matchedChildrenNodeList)const;
	private:
		vector<unique_ptr<HtmlPath>> m_htmlPathUptrList;
		const string m_tagName;
	};

	/**
	@brief �v�f�ԍ��i�q��j�̎w��B�C���[�W�́Atagname[n]�B
	*/
	class PositionHtmlPath :public HtmlPath{
	public:
		PositionHtmlPath(const std::size_t p_pos): m_position(p_pos){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	private:
		std::size_t m_position;
	};

	/**
	@brief �����i�q��j�w��B�C���[�W�́Atagname[@id="xxx"]
	*/
	class AttributesHtmlPath :public HtmlPath{
	public:
		///���C���h�J�[�h�Ŏw�肷��
		AttributesHtmlPath(const string& p_attrName, const string& p_attrVal)
			:  m_attrName(p_attrName), m_attrVal(p_attrVal){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	private:
		const string m_attrName;
		const string m_attrVal;
	};

	/**
	@brief �q���̃^�O�i//Descendant or self�j�����B�C���[�W�́A//tagname
	*/
	class DescendantsHtmlPath :public HtmlPath{
	public:
		DescendantsHtmlPath(){};
		///
		virtual unique_ptr<HtmlNodePtrs> filter(HtmlNodePtrs& p_nodePVec);
	protected:
		///���g�̃m�[�h���܂߂Ĕz���̃m�[�h�����ׂ�result�ɐݒ肷��
		void _filter(HtmlNodePtrs& p_result, const HtmlNodePtrs& p_nodePVec, std::map<const HtmlNode*, int>& p_alreadyRegNodeMap);
	private:
	};
	
	//------------------------------------
	/**
	@brief Html�p�X�̎��s�ҁB
	*/
	class HtmlPathExecutor: noncopyable{
	public:
		typedef HtmlPath::HtmlNodePtrs HtmlNodePtrs;
		virtual ~HtmlPathExecutor(){};
		/** �m�[�h�P�������Ɏ��s�i�ʏ�̓��[�g�m�[�h��n���j*/
		unique_ptr<HtmlNodePtrs> exec(const HtmlNode& p_node)const;
		/** �����̃m�[�h�������Ɏ��s�iHtmlPath���s���ʂ�����Ƀt�B���^�������ꍇ�ȂǂɎg�p�j*/
		unique_ptr<HtmlNodePtrs> exec(const HtmlNodePtrs& p_nodePtrs)const;
		/** �o�^�����A�N�Z�T�����ׂč폜���� */
		void clear(){ m_htmlPathVec.clear(); m_isTag = false; };
		/** �����Ńp�X�����R�ɒǉ����� */
		HtmlPathExecutor& add(HtmlPath* p_path);
		/** �q�����ׂĂ̎w��idescendant or self�j*/
		HtmlPathExecutor& slash2(){ add(new DescendantsHtmlPath()); m_isTag = false; return *this; };
		/**
		�^�O���̎w��
		@param tagName [in]�^�O��(���C���h�J�[�h�w��)
		*/
		HtmlPathExecutor& tag(const string& p_tagName);
		/**
		�q��F�����̎w��(attribute)�Btag()�ȊO�̌�Ɏg�p���Ă͂����Ȃ��B
		@param p_tagName [in]�^�O���i���C���h�J�[�h�w��j
		@param p_val [in]�l�i���C���h�J�[�h�w��j
		@exception runtime_exception tag()�̌�Ɏg�p���Ă��Ȃ��ꍇ�B
		*/
		HtmlPathExecutor& predAttr(const string& p_tagName, const string& p_val);
		/**
		�q��F�v�f�ԍ��̎w��iposition�j�Btag()�ȊO�̌�Ɏg�p���Ă͂����Ȃ��B
		@param p_index [in]�v�f�ԍ�
		@exception runtime_exception tag()�̌�Ɏg�p���Ă��Ȃ��ꍇ�B
		*/
		HtmlPathExecutor& predPos(const int p_index);
	private:
		vector<unique_ptr<HtmlPath>> m_htmlPathVec;
		bool m_isTag;
	};

}//namespace path

} //namespace nana


#endif  // #ifndef NANA_HTML_ANALYSIS_INCLUDED
