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
#ifndef NANA_HTML_DOCUMENT_INCLUDED
#define NANA_HTML_DOCUMENT_INCLUDED


#include <string>
#include <vector>
#include <list>
#include <map>
#include <algorithm>
#include <memory>
#include <ostream>
#include <istream>
#include <cstring>
#include <cstdlib>



namespace nana {

using std::string;
using std::vector;
using std::istream;
using std::unique_ptr;




/** c��HTML�̋󔒂��ǂ����`�F�b�N����BHTML4�d�l����9.Text�ɂ��ƈȉ���
HTML�ň����X�y�[�X�����A�Ōオ�ʓ|�Ȃ̂ŏ������B
ASCII space (&#x0020;)
ASCII tab (&#x0009;)
ASCII form feed (&#x000C;)
Zero-width space (&#x200B;)
*/
const bool IsSpace(const char& c);

inline const bool IsNotSpace(const char& c){
	return !IsSpace(c);
};


///�R�s�[�����Ȃ����߂�
class noncopyable{
protected:
	noncopyable(){};
	~noncopyable(){};
private:  // emphasize the following members are private
	noncopyable(const noncopyable&);
	noncopyable& operator=(const noncopyable&){};
};



/**
@breif <pre>
HTML�̍\���̂P��\���B�\���Ƃ͈ȉ��̂悤�ɒ�`����i��ʓI�ł͂Ȃ����Ƃɒ��Ӂj�B
�E�J�n�^�O
�E�I���^�O
�E�e�L�X�g
�E�R�����g
�E�錾�i!doctype�A?xml�j
</pre>*/
class HtmlPart : noncopyable{
public:
	enum Type { TEXT, TAG, COMMENT, /**�錾�i!doctype�A?xml�Ȃǁj*/DECLARATION, /**�^�O�̉E���́����Ȃ�*/NOT_END };
	HtmlPart(const std::string& p_str, const long p_line, const long p_pos)
		:m_contentStr(p_str), m_line(p_line), m_pos(p_pos) {};
	///
	virtual ~HtmlPart(){};
	///�v�f�̃^�C�v�i�e�L�X�g�A�^�O�Ȃǁj 
	virtual const Type type() const = 0;
	///�v�f�̃^�C�v�̕�����\�� 
	virtual const std::string& typeStr() const;
	///�^�O���i�^�C�v���^�O�łȂ��ꍇ�͋󕶎���Ԃ��j 
	virtual const string& tagName() const{
		static const string emp("");
		return emp;
	};
	///�ʒu�i�s���j
	virtual const long lineNum()const{ return m_line; };
	///�ʒu�i�擪����̃o�C�g���j
	virtual const long posNum()const{ return m_pos; };
	///�^�O�S�̂̕����� 
	virtual const std::string& str() const{ return m_contentStr; };
	///�������擾����i������Ȃ��ꍇ�A�^�C�v���^�O�łȂ��ꍇ�͋󕶎���Ԃ��j 
	virtual const std::string& attr(const std::string& p_key, const int p_index) const{
		static const std::string strNull("");
		return strNull;
	};
	///���������݂��邩 
	virtual const bool hasAttr(const string& p_key, const int& p_index) const{
		return false;
	};
	///�������̈ꗗ
	virtual std::unique_ptr<vector<const string*>> attrNames()const
	{ return unique_ptr<vector<const string*>>(new vector<const string*>); };
private:
	const std::string m_contentStr;
	const long m_line;
	const long m_pos;
};

//
std::ostream& operator << (std::ostream& os, const HtmlPart& htmlParts);


/**
@breif <pre>
HTML�̃^�O�v�f�������N���X�i��ʓI�ɂ�Node�̓e�L�X�g�Ȃǂ������������ł͈���Ȃ����ɒ��Ӂj�B
�����ɕێ�����^�O�i HtmlPart �j�͑��̃N���X�����̂������A�������Ǘ�����̂�
���̃N���X���ł̓|�C���^�����ێ����A��������s��Ȃ��B
���̃N���X���ɕێ����鎩�g�̃N���X�̎��͎̂��g�ŊǗ�����B
</pre>*/
class HtmlNode : noncopyable{
public:
	typedef std::vector<std::unique_ptr<HtmlNode>> NodeUptrs;
	typedef NodeUptrs::const_iterator const_iteraotr;
	HtmlNode()
		: m_startTagPartsPtr(nullptr), m_endTagPartsPtr(nullptr), m_parentNodePtr(nullptr)
	{ };
	//
	HtmlNode(const HtmlPart* p_Start, const HtmlPart* p_End, const HtmlNode* p_Parent)
	: m_startTagPartsPtr(p_Start), m_endTagPartsPtr(p_End), m_parentNodePtr(p_Parent){};
	///
	virtual ~HtmlNode(){};
	///�J�n�^�O�i���݂��Ȃ��ꍇ�Anullptr�j�B
	const HtmlPart* startTag()const{ return m_startTagPartsPtr; };
	///�I���^�O�i���݂��Ȃ��ꍇ�Anullptr�j
	const HtmlPart* endTag()const{ return m_endTagPartsPtr; };
	///�p�[�T�n���h���ȊO�͎g�p�֎~�B�����̓N���X�����Ŕp���̊Ǘ������Ȃ��B
	void setEndTag(const HtmlPart* p_endTag){ m_endTagPartsPtr = p_endTag; };
	void appendChild(std::unique_ptr<HtmlNode>&& p_Child){
		p_Child->m_parentNodePtr = this;
		m_childNodeUptrs.push_back(move(p_Child));
	};
	///�J�n�^�O�ƏI���^�O���Z�b�g�ő��݂��邩�i�^�O�����Ă��邩�H�j�B
	const bool isClosed()const{ return (m_startTagPartsPtr != nullptr && m_endTagPartsPtr != nullptr); };
	///�q�m�[�h�i childNodeList() �j�̊J�n�ʒu�C�e���[�^
	const_iteraotr begin() const{ return m_childNodeUptrs.begin(); };
	///�q�m�[�h�i childNodeList() �j�̍Ō�̎��̈ʒu�̃C�e���[�^
	const_iteraotr end() const{ return m_childNodeUptrs.end(); };
	///�q�m�[�h
	const NodeUptrs& childNodeList() const{ return m_childNodeUptrs; };
	///�e�m�[�h�̃|�C���^�B���݂��Ȃ��ꍇ�Anullptr�B
	const HtmlNode* parent() const{ return m_parentNodePtr; };
	///�^�O��
	const std::string& tagName()const;
	///�p�X
	const std::string pathStr()const;
	///�����ێ����Ă���J�n�ƏI���^�O�𕶎���o�́i�f�o�b�O�p�j 
	const string tagStr()const;
private:
	const HtmlPart* m_startTagPartsPtr;
	const HtmlPart* m_endTagPartsPtr;
	const HtmlNode* m_parentNodePtr;
	///�q�m�[�h�B
	NodeUptrs m_childNodeUptrs;
};


//
std::ostream& operator << (std::ostream& os, const HtmlNode& htmlNode);


/**
@breif <pre>
�^�O�̉�͂��������ʂ�ۑ�����N���X�B
��͈͂ȉ��̂��Ƃ����s��Ȃ��B����ȏ�̉�͂͑��̃N���X��֐��ōs���B
�E�R�����g�i���I--�j
�Edeclaration�i���Idoctype�A���H�j
�E�^�O�i��yyy ����/xxx����zzz/�����ꂼ��ʁX�Ɉ����j
�E�e�L�X�g�i��L�ȊO�̂��́j
</pre>
*/
class HtmlDocument: noncopyable{
public:
	typedef std::vector<unique_ptr<HtmlPart> > HtmlPartsUptrs;
	typedef HtmlPartsUptrs::const_iterator const_iterator;
	typedef vector<const HtmlPart*> SearchResults;
	typedef unique_ptr<SearchResults> SearchResultsUptr;
	//
	HtmlDocument(unique_ptr<HtmlPartsUptrs>&& p_partsUptr, unique_ptr<HtmlNode>&& p_rootNode)
	: m_stockedPartsUptrsUptr(move(p_partsUptr)), m_rootNodeUptr(move(p_rootNode)) {};
	///htmlPartsList() �̃C�e���[�^( unique_ptr<HtmlPart> )
	const_iterator begin() const{ return m_stockedPartsUptrsUptr->begin(); };
	const_iterator end() const{ return m_stockedPartsUptrsUptr->end(); };

	///HTML�̃^�O�̃��X�g�S��
	const HtmlPartsUptrs& htmlPartsList()const{ return *m_stockedPartsUptrsUptr; };
	///���[�g�m�[�h
	const HtmlNode& rootNode()const{ return *m_rootNodeUptr; };
	
	///�v�f�̐�
	const HtmlPartsUptrs::size_type size()const{ return m_stockedPartsUptrsUptr->size(); };
	///�ʒu���w�肵�ă^�O���擾����(������Ȃ��ꍇnullptr)
	const HtmlPart* at(const int p_index)const{
		if(p_index < m_stockedPartsUptrsUptr->size()) return (*m_stockedPartsUptrsUptr)[p_index].get();
		return nullptr;
	};
	/**�w��̃^�O��͈͌�������(start �� end�łȂ��Ƃ����Ȃ�)
	@param start [in]�͈͂̊J�n�B���̃|�C���^�Ɠ����|�C���^���J�n�ʒu�Ƃ���B
	@param end [in]�͈͂̏I���B���̃|�C���^�Ɠ����|�C���^���I���ʒu�Ƃ���B
	@return �w��͈̔͂̃p�[�c��Ԃ��B���݂��Ȃ��ꍇ�A�T�C�Y0�̃I�u�W�F�N�g��Ԃ�
	*/
	SearchResultsUptr range(
		const HtmlPart* p_start, const HtmlPart* p_end)const;
private:
	unique_ptr<HtmlPartsUptrs> m_stockedPartsUptrsUptr;
	unique_ptr<HtmlNode> m_rootNodeUptr;
};


class TagHtmlParts : public HtmlPart {
public:
	typedef std::map<std::string, vector<string> > AttrMap;
	TagHtmlParts(const string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){ parseTag(); };
	virtual ~TagHtmlParts(){};
	virtual const HtmlPart::Type type() const{ return TAG; };
	virtual const string& tagName()const{ return m_tagName; };
	virtual const std::string& attr(const std::string& p_key, const int p_index)const;
	virtual const bool hasAttr(const string& p_key, const int p_index)const;
	///�������̈ꗗ
	virtual std::unique_ptr<vector<const string*>> attrNames()const;
protected:
	void parseTag();
private:
	std::string m_tagName;
	AttrMap m_attrMap;
};



class TextHtmlParts : public HtmlPart {
public:
	TextHtmlParts(const string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return TEXT; };
};

class CommentHtmlParts : public HtmlPart {
public:
	CommentHtmlParts(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return COMMENT; };
};

///doctype��?xml�Ȃǂ�\���B
class DeclarationHtmlParts : public HtmlPart {
public:
	DeclarationHtmlParts(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return DECLARATION; };
};

///�^�O�̏I���i���j��������Ȃ��v�f��\�� 
class NotEndHtmlParts : public HtmlPart {
public:
	NotEndHtmlParts(const std::string& p_str, const long p_line, const long p_pos)
		:HtmlPart(p_str, p_line, p_pos){};
	virtual const HtmlPart::Type type() const{ return NOT_END; };
};

//HtmlParser�̃n���h��----------------------------
/**
@brief HTML�p�[�X�����邽�߂̃n���h���B�g�p����h���N���X�ŉ�͂̎d����ς��B
*/
class HtmlSaxParserHandler :noncopyable{
public:
	HtmlSaxParserHandler(){};
	virtual ~HtmlSaxParserHandler(){};
	///�J�n��m�点��B�n���h���̏������p�B 
	virtual void start() = 0;
	///�e�L�X�g�̏ꍇ��Sax����Ăяo�����B 
	virtual void text(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///�^�O�i�R�����g�ȊO�́����ł�����ꂽ���́j�̏ꍇ��Sax����Ăяo�����B 
	virtual void tag(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///�R�����g�i���I�|�|�|�|���j�̏ꍇ��Sax����Ăяo�����B
	virtual void comment(const std::string& p_str, const long p_line, const long p_pos) = 0;
	///�^�O�̏I���i���j���Ȃ��ꍇ��Sax����Ăяo�����B
	virtual void notEnd(const std::string& p_str, const long p_line, const long p_pos) = 0;
};


/**
@brief HTML����͂��A�^�O�z��( HtmlPart �̔z��)��Ԃ�HTML�p�[�T�n���h���B
*/
class SimpleHtmlSaxParserHandler :public HtmlSaxParserHandler {
public:
	SimpleHtmlSaxParserHandler(){};
	virtual ~SimpleHtmlSaxParserHandler(){};
	virtual void start(){
		m_resultPartsUptrsUptr = unique_ptr<HtmlDocument::HtmlPartsUptrs>(new HtmlDocument::HtmlPartsUptrs);
	};
	virtual void text(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new TextHtmlParts(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	};
	virtual void tag(const std::string& p_str, const long line, const long pos);
	virtual void comment(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new CommentHtmlParts(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	};
	virtual void notEnd(const std::string& p_str, const long p_line, const long p_pos){
		unique_ptr<HtmlPart> ptr(new NotEndHtmlParts(p_str, p_line, p_pos));
		m_resultPartsUptrsUptr->push_back(move(ptr));
	}
	//�p�[�X�������ʂ��擾����B���s�O�����ʎ擾���nullptr���Ԃ�B
	unique_ptr<HtmlDocument::HtmlPartsUptrs> result(){
		return move(m_resultPartsUptrsUptr);
	};

private:
	unique_ptr<HtmlDocument::HtmlPartsUptrs> m_resultPartsUptrsUptr;
};



//HtmlSaxParser-----------------------------------------------
/**
@brief HTML�p�[�T�B�h���N���X�͑��݂��Ȃ��B�g�p����n���h����ς��邱�Ƃŉ�͕��@��ς�����B
*/
class HtmlSaxParser :noncopyable{
public:
	void parse(std::istream& is, HtmlSaxParserHandler& handler);
protected:
	const bool copyUntilFind(string& p_str, istream& p_is, const char p_targetC);
	const bool copyUntilFindCommentClosed(string& p_str, istream& p_is);
	//�N���A�֐�(�ۊǕϐ�str�̃N���A�ƁA���݂̈ʒu�ƍs����ۊǂ���)
	inline void clearStr(string& str, long& line, long& pos){ str.clear(); line = m_line; pos = m_pos; };
private:
	long m_line, m_pos;//�s��, �ʒu�i�擪����̃o�C�g���j
};



} //namespace nana


#endif  // #ifndef NANA_HTML_DOCUMENT_INCLUDED