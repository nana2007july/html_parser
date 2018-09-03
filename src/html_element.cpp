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
#include <string>
#include <vector>
#include <map>
#include <algorithm>
#include <memory>
#include <cstring>
#include <cstdlib>


#include "html_element.hpp"

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
const bool IsSpace(const char& c){
	static const std::string space(" \t\n");
	return space.find_first_of(c) != std::string::npos;
};






///�v�f�̃^�C�v�̕�����\�� 
const std::string& HtmlPart::typeStr() const{
	static const std::string textStr("TEXT"), tagStr("TAG"), comStr("COMMENT"),
		docStr("DECLARATION"), hateStr("?");
	switch(this->type()){
	case HtmlPart::TEXT:
		return textStr;
	case HtmlPart::TAG:
		return tagStr;
	case HtmlPart::COMMENT:
		return comStr;
	case HtmlPart::DECLARATION:
		return docStr;
	default:
		return hateStr;
	}
};




//
std::ostream& operator << (std::ostream& p_os, const HtmlPart& p_htmlParts){
	p_os << "[" << p_htmlParts.typeStr() << "(" << p_htmlParts.lineNum() << ")]" << p_htmlParts.str();
	return p_os;
}



const std::string& HtmlNode::tagName()const{
	static const string strNull("[nullptr]"), strErr("[err]");
	if(m_startTagPartsPtr == nullptr && m_endTagPartsPtr == nullptr) return strNull;
	if(m_startTagPartsPtr == nullptr) return m_endTagPartsPtr->tagName();
	if(m_startTagPartsPtr->type() == HtmlPart::NOT_END) return strErr;
	return m_startTagPartsPtr->tagName();
};
//
const std::string HtmlNode::pathStr()const{
	const HtmlNode* nodeP = this->m_parentNodePtr;
	if(nodeP == nullptr) return "/";
	string str(this->tagName());
	for(int i = 0; i < 20; ++i, nodeP = nodeP->m_parentNodePtr){
		if(nodeP->m_parentNodePtr == nullptr) return "/" + str;
		str = nodeP->tagName() + "/" + str;
	}
	return ".../" + str;
};

//�����̊J�n�E�I���^�O�̕�����\�� 
const string HtmlNode::tagStr()const{
	string ret("(");
	if(m_startTagPartsPtr != NULL) ret += m_startTagPartsPtr->tagName();
	ret += ",";
	if(m_endTagPartsPtr != NULL) ret += m_endTagPartsPtr->tagName();
	ret += ")";
	return ret;
};


//
std::ostream& operator << (std::ostream& p_os, const HtmlNode& p_htmlNode){
	p_os << (void*)&p_htmlNode;
	p_os << "[" << p_htmlNode.pathStr() << "]" << p_htmlNode.tagStr();
	if(p_htmlNode.startTag() != nullptr) p_os << ":" << *p_htmlNode.startTag();
	else if(p_htmlNode.endTag() != nullptr) p_os << ":" << *p_htmlNode.endTag();
	return p_os;
}


//�w��̃^�O��͈͌�������(start �� end�łȂ��Ƃ����Ȃ�)
HtmlDocument::SearchResultsUptr HtmlDocument::range(const HtmlPart* p_start, const HtmlPart* p_end)const{
	auto i = m_stockedPartUptrsUptr->begin();
	//�J�n�ʒu�܂Ői�߂� 
	for(; i != m_stockedPartUptrsUptr->end() && (*i).get() != p_start; ++i){}
	//�ԋp�l�쐬 
	SearchResultsUptr htmlElementVecUPtr(new vector<const HtmlPart*>());
	for(; i != m_stockedPartUptrsUptr->end(); ++i){
		htmlElementVecUPtr->push_back(i->get());
		if(i->get() == p_end) break;
	}
	return move(htmlElementVecUPtr);
};

static void toLowerCaseStr(string& p_str){
	transform(p_str.begin(), p_str.end(), p_str.begin(), ::tolower);
}

const std::string& TagHtmlPart::attr(const std::string& p_key, const std::size_t p_index)const{
	static const string strNull("");
	AttrMap::const_iterator i = m_attrMap.find(p_key);
	if(i != m_attrMap.end()){
		if(p_index < (i->second).size()) return (i->second)[p_index];
		return strNull;
	}
	return strNull;
};

const bool TagHtmlPart::hasAttr(const std::string& p_key, const std::size_t p_index)const{
	auto i = m_attrMap.find(p_key);
	if(i != m_attrMap.end()){
		if(p_index < (i->second).size()) return true;
		return false;
	}
	return false;
};

///�������̈ꗗ
std::unique_ptr<vector<const std::string*>> TagHtmlPart::attrNames()const{
	unique_ptr<vector<const std::string*>> keyVecUptr(new vector<const std::string*>);
	for(auto i = m_attrMap.begin(); i != m_attrMap.end(); ++i) keyVecUptr->push_back(&(i->first));
	return move(keyVecUptr);

};

void TagHtmlPart::parseTag(){
	//HTML�����񒆂ō��̈ʒu�̏�Ԃ�\�� 
	enum Status { TAG, NONE, ATTR_KEY, ATTR_VAL };
	enum Status s = TAG;
	static const string notTag(" \n\t=>/");
	static const string notKey(" \n\t=>/");
	string::const_iterator end = str().end();
	string::const_iterator p;
	string key, val;
	//
	for(string::const_iterator i = str().begin(); i != end; ++i){
		switch(s){
		case TAG:
			//�^�O(��F"<tag")��3�����ڂ���^�O�̏I�������� 
			p = find_first_of(i + 2, end, notTag.begin(), notTag.end());
			m_tagName = string(&(*i) + 1, &(*p) - &(*i) - 1);
			toLowerCaseStr(m_tagName);
			s = NONE;
			i = --p;
			break;
		case NONE:
			//�󔒂ł͂Ȃ������܂ňʒu��i�߂� 
			i = find_if(i, end, IsNotSpace);
			s = NONE;
			//�����̊J�n�̏ꍇ
			if(*i != '>' && *i != '/'){
				s = ATTR_KEY;
				--i;
			}
			break;
		case ATTR_KEY:
			p = i;
			//�L�[���ł͂Ȃ������܂ňʒu��i�߂� 
			i = find_first_of(i, end, notKey.begin(), notKey.end());
			//�L�[���擾
			key = string(p, i);
			toLowerCaseStr(key);
			//�󔒂ł͂Ȃ������܂ňʒu��i�߂� 
			i = find_if(i, end, IsNotSpace);
			//�G���h�̏ꍇ�̓L�[�̂�
			if(i == end || *i == '>' || *i == '/'){
				m_attrMap[key].push_back("");
				s = NONE;
				--i;
				break;
			}
			//�C�R�[�������邩���`�F�b�N
			if(i != end && *i == '='){
				s = ATTR_VAL;
				++i;
				//�󔒂ł͂Ȃ������܂ňʒu��i�߂� 
				i = find_if(i, end, IsNotSpace);
			} else{
				//�L�[�݂̂̏ꍇ
				m_attrMap[key].push_back("");
				s = NONE;
			}
			--i;
			break;
		case ATTR_VAL:
			p = i;
			if(*i == '"'){
				++i;
				//"�̈ʒu�܂Ői�߂�
				i = find(i, end, '"');
				val = string(++p, i);
			} else if(*i == '\''){
				++i;
				//'�̈ʒu�܂Ői�߂�
				i = find(i, end, '\'');
				val = string(++p, i);
			} else {
				//�N�H�[�g�Ȃ��B�����l�ł͂Ȃ��Ƃ���܂Ői�߂�
				i = find_first_of(i, end, notKey.begin(), notKey.end());
				val = string(p, i);
			}
			if(i == end) --i;
			//
			m_attrMap[key].push_back(val);
			//
			s = NONE;
			break;
		}
	}
}



void SimpleHtmlSaxParserHandler::tag(const std::string& p_str, const long p_line, const long p_pos){
	unique_ptr<HtmlPart> ptr;
	//����
	if(p_str.size() < 10){
		//������������Ȃ��̂ŁAdoctype�͂��肦�Ȃ�
		//�ʏ�̃^�O�̏ꍇ
		ptr.reset(new TagHtmlPart(p_str, p_line, p_pos));
	} else{
		char lowerStr[11];
		transform(p_str.begin(), p_str.begin() + 9, lowerStr, ::tolower);
		if(p_str[1] == '?'){
			//�H�Ŏn�܂�ꍇ
			ptr.reset(new DeclarationHtmlPart(p_str, p_line, p_pos));
		}else if(strncmp(lowerStr, "<!doctype", 9) == 0 && IsSpace(p_str[9])){
			//doctype[��]�̏ꍇ
			ptr.reset(new DeclarationHtmlPart(p_str, p_line, p_pos));
		} else{
			//�ʏ�̃^�O�̏ꍇ
			ptr.reset(new TagHtmlPart(p_str, p_line, p_pos));
		}
	}
	m_resultPartsUptrsUptr->push_back(move(ptr));
};




//HtmlSaxParser-----------------------------------------------

const bool HtmlSaxParser::copyUntilFind(string& p_str, istream& p_is, const char p_targetC){
	char c;
	while(p_is.get(c)){
		++m_pos;
		if(c == '\n') ++m_line;
		//
		p_str += c;
		if(c == p_targetC) return true;;
	}
	return false;
}

const bool HtmlSaxParser::copyUntilFindCommentClosed(string& p_str, istream& p_is){
	while(true){
		copyUntilFind(p_str, p_is, '>');
		if(strncmp(p_str.data() + p_str.size() - 3, "-->", 3) == 0) return true;
		if(!p_is) return false;
	}
	return false;
}


void HtmlSaxParser::parse(std::istream& p_is, HtmlSaxParserHandler& p_handler){
	string str("");
	char c_c2[] = {'\0', '\0', '\0'};//c��c2��A������������
	char &c = c_c2[0]; //c_c2��1�����ڂƌ��т���
	char &c2 = c_c2[1]; //c_c2��2�����ڂƌ��т���
	long line = 1, pos = 0; //str������ۊǊJ�n�ʒu�ł̉��s�̐�
	m_line = 1; //�J�����g�ʒu�ł̉��s�̐�
	m_pos = 0;

	//�J�n
	p_handler.start();

	//���[�v 
	while(p_is.get(c)){
		++m_pos;
		if(c == '\n') ++m_line;
		//
		switch(c){
		case '<':
			if(!p_is.get(c2)){
				//�X�g���[���̏I���
				str += c;
				break;
			}
			if(IsSpace(c2)){
				//c2���󔒂̏ꍇ("< ")�A�^�O�ł͂Ȃ��̂Ŏ��ɍs�� 
				str += c_c2;
				continue;
			}
			//���߂���������n���h���ɓn��
			if(!str.empty()){
				p_handler.text(str, line, pos);
				clearStr(str, line, pos);
			}

			//�V���ȊJ�n������ݒ� 
			str = c_c2;

			//�^�O�̏I���܂ŃR�s�[ 
			if(!copyUntilFind(str, p_is, '>')){
				//�^�O�̏I���i���j��������Ȃ�
				p_handler.notEnd(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			if(str.size() < 6){
				//tag�œo�^�B�����������Ȃ����ăR�����g�^�O�͂��肦�Ȃ�
				p_handler.tag(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			if(strncmp(str.data(), "<!--", 4) != 0){
				//���Ŏn�܂��Ă��邪�R�����g�ł͂Ȃ��̂ŁA�^�O�Ɣ��f
				p_handler.tag(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//�R�����g�̏ꍇ�Bp_str�̏I��肪�R�����g�̏I��肩�`�F�b�N
			if(strncmp(str.data() + str.size() - 3, "-->", 3) == 0){
				//�R�����g�̏I��肪���݂���ꍇ
				p_handler.comment(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//�R�����g�̏I��肪������Ȃ��̂Ō�����܂ŒT�� 
			if(copyUntilFindCommentClosed(str, p_is)){
				//�R�����g�̏I��肪���������ꍇ
				p_handler.comment(str, line, pos);
				clearStr(str, line, pos);
				continue;
			}
			//�R�����g�̏I����������Ȃ��ꍇ
			p_handler.notEnd(str, line, pos);
			clearStr(str, line, pos);
			continue;

			break;
		default:
			str += c;
			break;
		}
	}
	//�n���h���ɓn��
	if(!str.empty()){
		p_handler.text(str, line, pos);
	}
};




} //namespace nana

