<?xml version="1.0" encoding="UTF-8"?>

<xsl:stylesheet xmlns:xsl="http://www.w3.org/1999/XSL/Transform" version="1.0">
	<xsl:import href="transform_base.xsl"/>

	<xsl:output method="xml" indent="yes"/>

	<xsl:param name="keyword.namespace" select="'Atmel.Language.C'"/>

	<xsl:template name="generate.book.title">
		<xsl:text>LUFA Library</xsl:text>
	</xsl:template>

	<xsl:template name="generate.book.id">
		<xsl:param name="book.title"/>
		<xsl:choose>
			<xsl:when test="@id">
				<xsl:value-of select="@id"/>
			</xsl:when>
			<xsl:otherwise>
				<xsl:value-of select="translate($book.title, ' ','')"/>
			</xsl:otherwise>
		</xsl:choose>
	</xsl:template>

	<xsl:template match="doxygen">
		<xsl:variable name="book.title">
			<xsl:call-template name="generate.book.title"/>
		</xsl:variable>
		<xsl:variable name="book.id">
			<xsl:call-template name="generate.book.id">
				<xsl:with-param name="book.title" select="$book.title"/>
			</xsl:call-template>
		</xsl:variable>

		<book name="{$book.id}">
			<title>
				<xsl:value-of select="$book.title"/>
			</title>

			<xsl:call-template name="generate.top.level.page">
				<xsl:with-param name="top.level.page" select="compounddef[@kind = 'page' and contains(@id, 'index')]"/>
			</xsl:call-template>

			<xsl:for-each select="compounddef[@kind = 'page' and not(contains(@id, 'index'))]">
				<xsl:if test="not(//innerpage[@refid = current()/@id])">
					<xsl:call-template name="generate.top.level.page">
						<xsl:with-param name="top.level.page" select="current()"/>
					</xsl:call-template>
				</xsl:if>
			</xsl:for-each>
		</book>
	</xsl:template>

	<xsl:template name="generate.top.level.page">
		<xsl:param name="top.level.page"/>

		<chapter id="{$top.level.page/@id}">
			<title>
				<xsl:value-of select="$top.level.page/title"/>
			</title>
			<xsl:apply-templates select="$top.level.page/detaileddescription"/>

			<xsl:for-each select="$top.level.page/innerpage">
				<xsl:apply-templates select="ancestor::*/compounddef[@kind = 'page' and @id = current()/@refid]"/>
			</xsl:for-each>
		</chapter>
	</xsl:template>

	<xsl:template match="compounddef[@kind = 'page']">
		<section id="{@id}">
			<title>
				<xsl:value-of select="title"/>
			</title>
			<xsl:apply-templates select="detaileddescription"/>

			<xsl:for-each select="innerpage">
				<xsl:apply-templates select="ancestor::*/compounddef[@kind = 'page' and @id = current()/@refid]"/>
			</xsl:for-each>
		</section>
	</xsl:template>

	<xsl:template match="linebreak">
		<xsl:text>&#10;</xsl:text>
	</xsl:template>

	<xsl:template match="image">
		<figure>
			<title>
				<xsl:value-of select="."/>
			</title>

			<mediaobject>
				<imageobject>
					<imagedata>
						<xsl:attribute name="fileref">images/<xsl:value-of select="@name"/></xsl:attribute>
					</imagedata>
				</imageobject>
			</mediaobject>
		</figure>
	</xsl:template>

	<xsl:template match="detaileddescription">
		<xsl:apply-templates/>
	</xsl:template>

	<xsl:template match="sect1 | sect2 | sect3 | sect4 | sect5 | sect6 | sect7 | sect8 | sect9">
		<section>
			<xsl:if test="@id">
				<xsl:attribute name="id">
					<xsl:value-of select="@id"/>
				</xsl:attribute>
			</xsl:if>

			<title>
				<xsl:value-of select="title"/>
			</title>

			<xsl:apply-templates/>
		</section>
	</xsl:template>

	<xsl:template match="anchor">
		<xsl:if test="@id">
			<indexterm id="{@id}"/>
		</xsl:if>
	</xsl:template>

	<xsl:template match="compoundname"/>

	<xsl:template match="title"/>

	<xsl:template match="indexterm"/>

	<xsl:template match="indexentry">
		<xsl:variable name="prim">
			<xsl:choose>
				<xsl:when test="contains(primaryie, ',')">
					<xsl:value-of select="normalize-space(substring-before(primaryie, ','))"/>
				</xsl:when>
				<xsl:otherwise>
					<xsl:value-of select="primaryie"/>
				</xsl:otherwise>
			</xsl:choose>
		</xsl:variable>

		<xsl:variable name="sec">
			<xsl:choose>
				<xsl:when test="contains(primaryie, ',')">
					<xsl:value-of select="normalize-space(substring-after(primaryie, ','))"/>
				</xsl:when>
				<xsl:when test="seondariye">
					<xsl:value-of select="secondaryie"/>
				</xsl:when>
				<xsl:otherwise/>
			</xsl:choose>
		</xsl:variable>

		<xsl:variable name="tert">
			<xsl:choose>
				<xsl:when test="contains(primaryie, ',')">
					<xsl:choose>
						<xsl:when test="secondaryie">
							<xsl:value-of select="secondaryie"/>
						</xsl:when>
					</xsl:choose>
				</xsl:when>
				<xsl:when test="tertiaryie">
					<xsl:value-of select="tertiaryie"/>
				</xsl:when>
				<xsl:otherwise/>
			</xsl:choose>
		</xsl:variable>

		<indexterm>
			<xsl:if test="@id">
				<xsl:attribute name="id">
					<xsl:value-of select="@id"/>
				</xsl:attribute>
			</xsl:if>
			<xsl:if test="$prim">
				<primary>
					<xsl:value-of select="$prim"/>
				</primary>
			</xsl:if>
			<xsl:if test="$sec">
				<secondary>
					<xsl:value-of select="$sec"/>
				</secondary>
			</xsl:if>
			<xsl:if test="$tert">
				<tertiary>
					<xsl:value-of select="$tert"/>
				</tertiary>
			</xsl:if>
		</indexterm>
	</xsl:template>
</xsl:stylesheet>
