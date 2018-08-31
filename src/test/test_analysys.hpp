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
#include <iostream>
#include <sstream>



#include "assert.hpp"



#include "../html_analysys.hpp"

namespace{

using namespace std;
using namespace nana::test;



//------------------------------------------
///�K�w�Ȃ��ꍇ
TEST_FUNC(test_wildcardMatch1){
	A_EQUALS(nana::wildcardMatch("t*t", "1t2t"), false, "���S�}�b�`����m�F");
	A_EQUALS(nana::wildcardMatch("t*t", "t2tee"), false, "���S�}�b�`����m�F");
	A_EQUALS(nana::wildcardMatch("t*t", "t244hgfn2t"), true, "���S�}�b�`����m�F");
	A_EQUALS(nana::wildcardMatch("t\\*t", "t222t"), false, "�G�X�P�[�v�̊m�F");
	A_EQUALS(nana::wildcardMatch("t\\*t", "t*t"), true, "�G�X�P�[�v�̊m�F");
	A_EQUALS(nana::wildcardMatch("tt*", "ttbmn,123254"), true, "�Ō�Ƀ��C���h�w��m�F");
	A_EQUALS(nana::wildcardMatch("tt\\*", "tt*"), true, "�Ō�ɃG�X�P�[�v�m�F");
	A_EQUALS(nana::wildcardMatch("tt\\*", "ttdagaga"), false, "�Ō�̃G�X�P�[�v�m�F");
	A_EQUALS(nana::wildcardMatch("tt\\", "tt"), true, "�Ō��\\�m�F");
	A_EQUALS(nana::wildcardMatch("t?t", "t2t"), true, "���C���h�m�F");
	A_EQUALS(nana::wildcardMatch("t?t", "t22t"), false, "���C���h�m�F");
	A_EQUALS(nana::wildcardMatch("tt?", "tt"), false, "���C���h�m�F");
}


//------------------------------------------

///�K�w���Ȃ��ꍇ
TEST_FUNC(test_DocumentHtmlSaxParserHandler1){
	string str(" <!Doctype afdafa><tAg aA='xX'> <!--d/--></tag><tes2/><not gg='");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	A_EQUALS(node.childNodeList().size(), 3, "�^�O�̐�");
	//
	A_TRUE(node.parent() == nullptr, "���[�g�̐e�A�h���X");
	A_EQUALS(node.pathStr(), "/", "���[�g�̃p�X������");
	//
	A_EQUALS(node.childNodeList()[0]->childNodeList().size(), 0, "�q�m�[�h�̐�");
	A_EQUALS(node.childNodeList()[0]->parent(), &node, "�e");
	A_EQUALS(node.childNodeList()[0]->pathStr(), "/tag", "�p�X������");
	A_EQUALS(node.childNodeList()[0]->tagName(), "tag", "�^�O��");
	//
	A_EQUALS(node.childNodeList()[1]->childNodeList().size(), 0, "�p�X������");
	A_EQUALS(node.childNodeList()[1]->parent(), &node, "�e");
	A_EQUALS(node.childNodeList()[1]->pathStr(), "/tes2", "�p�X������");
	A_EQUALS(node.childNodeList()[1]->tagName(), "tes2", "�^�O��");
	//
	A_EQUALS(node.childNodeList()[2]->childNodeList().size(), 0, "�p�X������");
	A_EQUALS(node.childNodeList()[2]->parent(), &node, "�e");
	A_EQUALS(node.childNodeList()[2]->pathStr(), "/[err]", "�p�X������");
	A_EQUALS(node.childNodeList()[2]->tagName(), "[err]", "�^�O��");
};


///�K�w������ꍇ
TEST_FUNC(test_DocumentHtmlSaxParserHandler2){
	string str("<tag aA='xX'><tag2><div class>aaa</div></tag2><input type=\"hidden\"></tag><tes2/>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	A_EQUALS(node.childNodeList().size(), 2, "�^�O�̐�");
	//
	A_TRUE(node.parent() == nullptr, "���[�g�̐e�A�h���X");
	A_EQUALS(node.pathStr(), "/", "���[�g�̃p�X������");
	//
	nana::HtmlNode& tag = *node.childNodeList()[0];
	//
	A_EQUALS(tag.childNodeList().size(), 2, "�q�m�[�h�̐�");
	A_EQUALS(tag.parent(), &node, "�e");
	A_EQUALS(tag.pathStr(), "/tag", "�p�X������");
	A_EQUALS(tag.tagName(), "tag", "�^�O��");
	//
	A_EQUALS(tag.childNodeList()[1]->pathStr(), "/tag/input", "�p�X������");
	A_EQUALS(tag.childNodeList()[1]->tagName(), "input", "�^�O��");
	A_NOT_NULL(tag.childNodeList()[1]->startTag(), "�J�n�^�O�̑���");

	//
	nana::HtmlNode& tag2 = *tag.childNodeList()[0];
	//
	A_EQUALS(tag2.childNodeList().size(), 1, "�q�m�[�h�̐�");
	A_EQUALS(tag2.parent(), &tag, "�e");
	A_EQUALS(tag2.pathStr(), "/tag/tag2", "�p�X������");
	A_EQUALS(tag2.tagName(), "tag2", "�^�O��");
	//
	A_EQUALS(tag2.childNodeList()[0]->childNodeList().size(), 0, "�q�m�[�h�̐�");
	A_EQUALS(tag2.childNodeList()[0]->parent(), &tag2, "�e");
	A_EQUALS(tag2.childNodeList()[0]->pathStr(), "/tag/tag2/div", "�p�X������");
	A_EQUALS(tag2.childNodeList()[0]->tagName(), "div", "�^�O��");
};



///HTML�̃^�O�Ő������ꍇ
TEST_FUNC(test_DocumentHtmlSaxParserHandler3){
	string str("<html>\n<script><!-- function{if(i < 0) return 0;} --></script>\n<div><input type=\"hiddne\"><img ><img src/><a href=''>aaa</a></div></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	const nana::HtmlNode& htmlNode = *node.childNodeList()[0];
	for(auto i = htmlNode.begin(); i != htmlNode.end(); ++i){
		A_TRUE((*i)->isClosed(), "���Ă��邩�H");
	}

	//
	const nana::HtmlNode& scriptNode = *node.childNodeList()[0]->childNodeList()[0];
	A_EQUALS(scriptNode.tagName(), "script", "�^�O�`�F�b�N");
	nana::HtmlDocument::SearchResultsUptr partsVecUptr =  docUptr->range(scriptNode.startTag(), scriptNode.endTag());
	A_EQUALS(partsVecUptr->size(), 3, "parts�`�F�b�N");
	A_EQUALS((*partsVecUptr)[1]->str(), "<!-- function{if(i < 0) return 0;} -->", "parts�`�F�b�N");

};


///HTML�̃^�O�Ő������Ȃ��ꍇ
TEST_FUNC(test_DocumentHtmlSaxParserHandler4){
	string str("<html><div><a href=''>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	const nana::HtmlNode& node = docUptr->rootNode();
	//
	for(auto i = node.begin(); i != node.end(); ++i){
		A_FALSE((*i)->isClosed(), "���Ă��邩�H");
	}

};



//------------------------------------------
///�K�w�Ȃ��ꍇ
TEST_FUNC(test_HtmlPath1){
	string str("<html><div class=\"1\"/><br><div class=\"2\"/></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::path::HtmlPathExecutor executor;
	executor.tag("html").tag("div").predPos(0);
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec).size(), 1, "���o��");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div", "path()�e�X�g");
	A_EQUALS((*retExec)[0]->startTag()->str(), "<div class=\"1\"/>", "���o�^�O�̊m�F");

	//
	nana::path::HtmlPathExecutor executor2;
	executor2.slash2().tag("br");
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec2 = executor2.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec2).size(), 1, "���o��");
	A_EQUALS((*retExec2)[0]->pathStr(), "/html/br", "path()�e�X�g");
	A_EQUALS((*retExec2)[0]->startTag()->str(), "<br>", "���o�^�O�̊m�F");
	
	//�q���tag()�̌�Ɏg�p���Ȃ��Ƃ����Ȃ�
	try{
		executor2.slash2().predAttr("*", "");
		A_TRUE(false, "��O���������Ă��Ȃ�");
	}catch(std::runtime_error& e){
		//������
		A_NOT_NULL(e.what(), "����������");
	}catch(std::exception){
		A_TRUE(false, "runtime_exception���������Ȃ��Ƃ����Ȃ��̂ɔ������Ă��Ȃ�");
	}
	try{
		executor2.slash2().predPos(0);
		A_TRUE(false, "��O���������Ă��Ȃ�");
	} catch(std::runtime_error& e){
		//������
		A_NOT_NULL(e.what(), "����������");
	} catch(std::exception){
		A_TRUE(false, "runtime_exception���������Ȃ��Ƃ����Ȃ��̂ɔ������Ă��Ȃ�");
	}
};



///�K�w����ꍇ
TEST_FUNC(test_HtmlPath2){
	string str("<html><div id='main'><form name='f'><input name='1'></form></div><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::path::HtmlPathExecutor executor;
	executor.slash2().tag("div").predAttr("id", "main").slash2().tag("input");
	unique_ptr<nana::path::HtmlPath::HtmlNodePtrs> retExec = executor.exec(docUptr->rootNode());
	//
	A_EQUALS((*retExec).size(), 1, "���o��");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div/form/input", "path()�e�X�g");
	A_EQUALS((*retExec)[0]->startTag()->str(), "<input name='1'>", "���o�^�O�̊m�F");

	//�p�X���s���ʂɂ���Ƀp�X�̒T��������ꍇ
	executor.clear();
	executor.slash2().tag("div");
	//
	retExec = executor.exec(docUptr->rootNode());
	//
	executor.clear();
	executor.tag("form").tag("input");
	retExec = executor.exec(*retExec);
	//
	A_EQUALS((*retExec).size(), 1, "���o��");
	A_EQUALS((*retExec)[0]->pathStr(), "/html/div/form/input", "path()�e�X�g");

};




///�K�w����B�^�O�����Ă��邪����Ⴂ�ɂȂ��Ă���ꍇ
TEST_FUNC(test_EndTagAccessor1){
	string str("<html><div id='main'><form name='f'><input name='1'></div></form><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	auto nonClosedUptr = acc.nonClosedResult();
	auto alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 0, "���o��");
	A_EQUALS(alternatedUptr->size(), 1, "���o��");
	A_EQUALS((*alternatedUptr)[0]->pathStr(), "/html/div/form//div", "path()�e�X�g");
};


///�K�w����B���^�O���Ȃ��ꍇ
TEST_FUNC(test_EndTagAccessor2){
	string str("<html><div id='main'><form name='f'><input name='1'></div><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::EndTagAccessor::SearchResultsUptr alternatedUptr, nonClosedUptr;
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	nonClosedUptr = acc.nonClosedResult();
	alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 1, "���o��");
	A_EQUALS(alternatedUptr->size(), 0, "���o��");
	A_EQUALS((*nonClosedUptr)[0]->pathStr(), "/html/div/form", "path()�e�X�g");
};


///�K�w����B���^�O���Ȃ��ꍇ
TEST_FUNC(test_CompositeAccessor1){
	string str("<html><div id='main'><form name='f'><input name='1'></div><blink></blink><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::CompositeAccessor acc;
	acc.add(new nana::EndTagAccessor)
	   .add(new nana::DeprecatedInHtml5Accessor);
	nana::HtmlNodeVisitor vis;
	//
	vis.access(docUptr->rootNode(), acc);
	//
	auto nonClosedUptr = acc.accessor<nana::EndTagAccessor>(0).nonClosedResult();
	auto deprecatedUptr = acc.accessor<nana::DeprecatedInHtml5Accessor>(1).result();
	//
	A_EQUALS(nonClosedUptr->size(), 1, "���o��");
	A_EQUALS((*nonClosedUptr)[0]->pathStr(), "/html/div/form", "path()�e�X�g");
	A_EQUALS(deprecatedUptr->size(), 1, "���o��");
	A_EQUALS((*deprecatedUptr)[0]->pathStr(), "/html/div/form/blink", "path()�e�X�g");
};


/**
HtmlSaxParser::copyUntilFindCommentClosed�̃o�O�C���̃e�X�g
�R�����g���J�n������A���^�O��2�����ƃR�����g�̏I����F���ł��Ȃ��o�O�B
*/
TEST_FUNC(test_copyUntilFindCommentClosed){
	string str("<html><!--<form name='f'><input name='1'><--><blink></blink><input name='2'></html>");

	nana::HtmlSaxParser parser;
	nana::DocumentHtmlSaxParserHandler handler;
	istringstream is(str);
	parser.parse(is, handler);
	unique_ptr<nana::HtmlDocument> docUptr = handler.result();

	//
	nana::EndTagAccessor acc;
	nana::HtmlNodeVisitor vis;

	//�`�F�b�N���s
	vis.access(docUptr->rootNode(), acc);

	//���ʎ擾(���ʂ̌^�� unique_ptr<vector<const HtmlNode*>>)
	auto nonClosedUptr = acc.nonClosedResult();
	auto alternatedUptr = acc.alternatedResult();
	//
	A_EQUALS(nonClosedUptr->size(), 0, "���o��");
	A_EQUALS(alternatedUptr->size(), 0, "���o��");
};


} //namespace
