<?xml version="1.0" encoding="utf-8" ?>
<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
  <xsl:output method="text" encoding="iso-8859-1"/>
  <xsl:template match="descriptors">
    <xsl:variable name="num_lang" select="count(languages/lang)"/>
    <xsl:text>#include "string-descriptors.h"&#10;</xsl:text>
    
    <!-- string descriptors -->
    <xsl:for-each select="strings/string/lang">
      <xsl:text>static const struct {&#10;</xsl:text>
      <xsl:text>  struct usb_st_string_descriptor base;&#10;</xsl:text>
      <xsl:text>  Uint16 chars[</xsl:text>
      <xsl:value-of select="string-length(text()) -1"/>
      <xsl:text>];&#10;</xsl:text>
      <xsl:text>} string_descriptor_</xsl:text>
      <xsl:number count="/descriptors/strings/string" format="1"/>
      <xsl:text>_</xsl:text>
      <xsl:choose>
	<xsl:when test="@id">
	  <xsl:value-of select="@id"/>
	</xsl:when>
	<xsl:otherwise>
	  <xsl:text>all</xsl:text>
	</xsl:otherwise>
      </xsl:choose>
      <xsl:text>= {{</xsl:text>
      <xsl:value-of select="string-length(text())*2 + 2"/>
      <xsl:text>, 3, {'</xsl:text>
      <xsl:value-of select="substring(text(), 1,1)"/>
      <xsl:text>'}}, {&#10;</xsl:text>
      <xsl:call-template name="output-UCS2">
	<xsl:with-param name="string" select="substring(text(), 2)"/>
      </xsl:call-template>
      <xsl:text>}};&#10;</xsl:text>
    </xsl:for-each>

    <!-- string tables -->
    <xsl:for-each select="/descriptors/languages/lang">
      <xsl:variable name="id" select="@id"/>
      <xsl:text>static const struct usb_st_string_descriptor * string_table_</xsl:text>
      <xsl:value-of select="$id"/>
      <xsl:text>[] =&#10;{&#10;</xsl:text>
      <xsl:for-each select="/descriptors/strings/string">
	<xsl:text>  &amp;string_descriptor_</xsl:text>
	<xsl:number count="/descriptors/strings/string" format="1"/>
	<xsl:text>_</xsl:text>
 	<xsl:choose>
	  <xsl:when test="lang[@id = $id]">
	    <xsl:value-of select="$id"/>
	  </xsl:when>
	  <xsl:when test="lang[@id = 'all' or count(@id) = 0]">
	    <xsl:text>all</xsl:text>
	  </xsl:when>
	  <xsl:otherwise>
	    <xsl:message terminate="yes">
	      <xsl:text>No string found for index </xsl:text>
	      <xsl:number count="/descriptors/strings/string" format="1"/>
	      <xsl:text> and language </xsl:text>
	      <xsl:value-of select="$id"/>
	    </xsl:message>
	  </xsl:otherwise>
	</xsl:choose>
	<xsl:text>.base,&#10;</xsl:text>
      </xsl:for-each>
      <xsl:text>};&#10;</xsl:text>
    </xsl:for-each>

    <!-- language descriptor -->
    <xsl:text>static const struct {&#10;</xsl:text>
    <xsl:text>  struct usb_st_language_descriptor base;&#10;</xsl:text>
    <xsl:text>  Uint16 langs[</xsl:text>
      <xsl:value-of select="$num_lang -1"/>
    <xsl:text>];&#10;</xsl:text>
    <xsl:text>} language_descriptor =&#10;{&#10;</xsl:text>
    <xsl:text>  {</xsl:text>
    <xsl:value-of select="$num_lang*2 + 2"/>
    <xsl:text>, 3, {</xsl:text>
    <xsl:value-of select="languages/lang[1]/text()"/>
    <xsl:text>}},&#10;  {</xsl:text>
    <xsl:for-each select="languages/lang[position() > 1]">
      <xsl:value-of select="text()"/>
      <xsl:text>, </xsl:text>
    </xsl:for-each>
    <xsl:text>}};&#10;</xsl:text>

    <!-- language lookup table -->
    <xsl:text>static const struct {&#10;</xsl:text>
    <xsl:text>  struct usb_st_string_languages base;&#10;</xsl:text>
    <xsl:text>  struct  usb_st_string_language_map map[</xsl:text>
    <xsl:value-of select="$num_lang - 1"/>
    <xsl:text>];&#10;} </xsl:text> 
    <xsl:text>string_languages_full={{</xsl:text>
    <xsl:value-of select="$num_lang"/>
    <xsl:text>, </xsl:text> 
    <xsl:value-of select="count(strings/string)"/>
    <xsl:text>, &amp;language_descriptor.base</xsl:text>
    <xsl:text>, &#10;    {{</xsl:text> 
    <xsl:value-of select="languages/lang[1]/text()"/>
    <xsl:text>, string_table_</xsl:text> 
    <xsl:value-of select="languages/lang[1]/@id"/>
    <xsl:text>}}}, {&#10;</xsl:text>
    <xsl:for-each select="languages/lang[position() > 1]">
      <xsl:text>    {</xsl:text>
      <xsl:value-of select="text()"/>
      <xsl:text>, string_table_</xsl:text>
      <xsl:value-of select="@id"/>
      <xsl:text>},&#10;</xsl:text>
    </xsl:for-each>
    <xsl:text>  }&#10;};&#10;</xsl:text>
    <xsl:text>const struct usb_st_string_languages * const string_languages = &amp;string_languages_full.base;&#10;</xsl:text>
  </xsl:template>

  <xsl:template name="output-UCS2">
    <xsl:param name="string"/>
    <xsl:if test="string-length($string) &gt; 0">
      <xsl:text>'</xsl:text>
      <xsl:value-of select="substring($string, 1,1)"/>
      <xsl:text>'</xsl:text>
      <xsl:if test="string-length($string) &gt; 1">
	<xsl:text>, </xsl:text>
      </xsl:if>
      <xsl:call-template name="output-UCS2">
	<xsl:with-param name="string" select="substring($string, 2)"/>
      </xsl:call-template>
    </xsl:if>
  </xsl:template>
</xsl:stylesheet>


