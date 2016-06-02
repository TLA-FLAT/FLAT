/*
 * Copyright (C) 2015 menzowi
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
package nl.mpi.tla.flat.deposit.util;

import java.io.File;
import java.io.InputStream;
import java.net.URL;
import java.util.Iterator;
import java.util.Map;
import java.util.regex.Matcher;
import java.util.regex.Pattern;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.transform.Source;
import net.sf.saxon.s9api.DocumentBuilder;
import net.sf.saxon.s9api.Processor;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.SaxonApiException;
import net.sf.saxon.s9api.XPathCompiler;
import net.sf.saxon.s9api.XPathSelector;
import net.sf.saxon.s9api.XQueryCompiler;
import net.sf.saxon.s9api.XdmItem;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XdmValue;
import net.sf.saxon.s9api.XsltCompiler;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
import net.sf.saxon.tree.wrapper.VirtualNode;
import org.w3c.dom.Document;
import org.w3c.dom.Node;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 *
 * @author menzowi
 */
public class Saxon {
    
    private static final Logger logger = LoggerFactory.getLogger(Saxon.class.getName());
    
    /**
     * The Saxon processor from which there should be only one. Any Saxon
     * related instance, e.g., an XML document or an XSLT transform, should
     * share this processor. Otherwise Saxon will complain as it can't used
     * shared constructs, like the NamePool.
     */
    static private Processor sxProcessor = null;
    /**
     * The Saxon XSLT compiler.
     */
    static private XsltCompiler sxXsltCompiler = null;
    /**
     * The Saxon XPath compiler.
     */
    static private XPathCompiler sxXPathCompiler = null;
    /**
     * The Saxon XQuery compiler.
     */
    static private XQueryCompiler sxXQueryCompiler = null;
    /**
     * The Saxon Document Builder
     */
    static private DocumentBuilder sxDocumentBuilder = null;

    /**
     * Get a Saxon processor, i.e., just-in-time create the Singleton.
     *
     * @return The Saxon processor
     */
    public static synchronized Processor getProcessor() {
        if (sxProcessor == null) {
            sxProcessor = new Processor(false);
            try {
                SaxonExtensionFunctions.registerAll(sxProcessor.getUnderlyingConfiguration());
            } catch (Exception e) {
                logger.error("Couldn't register the Saxon extension functions!",e);
            }
        }
        return sxProcessor;
    }

    private static synchronized XsltCompiler getXsltCompiler() {
        if (sxXsltCompiler == null) {
            sxXsltCompiler = getProcessor().newXsltCompiler();
        }
        return sxXsltCompiler;
    }

    private static synchronized XQueryCompiler getXQueryCompiler() {
        if (sxXQueryCompiler == null) {
            sxXQueryCompiler = getProcessor().newXQueryCompiler();
        }
        return sxXQueryCompiler;
    }

    private static synchronized DocumentBuilder getDocumentBuilder() {
        if (sxDocumentBuilder == null) {
            sxDocumentBuilder = getProcessor().newDocumentBuilder();
        }
        return sxDocumentBuilder;
    }

    /**
     * Load an XML document.
     *
     * @param src The source of the document.
     * @return A Saxon XDM node
     * @throws SaxonApiException
     */
    static public XdmNode buildDocument(Source src) throws SaxonApiException {
        return getDocumentBuilder().build(src);
    }

    /**
     * Load an XML into a DOM.
     *
     * @param src The source of the document.
     * @return A DOM document node
     * @throws Exception
     */
    static public Document buildDOM(File src) throws Exception {
        Document doc = DocumentBuilderFactory.newInstance().newDocumentBuilder().parse(src);
        doc.setDocumentURI(src.toURI().toString());
        return doc;
    }

    /**
     * Compile an XLST document. To use compiled XSLT document use the load()
     * method to turn it into a XsltTransformer.
     *
     * @param xslStylesheet
     * @return An Saxon XSLT executable, which can be shared.
     * @throws SaxonApiException
     */
    static public XsltExecutable buildTransformer(XdmNode xslStylesheet) throws SaxonApiException {
        return getXsltCompiler().compile(xslStylesheet.asSource());
    }

    /**
     * Convenience method to build a XSLT transformer from a resource.
     *
     * @param uri The location of the resource
     * @return An executable XSLT
     * @throws Exception
     */
    static public XsltExecutable buildTransformer(File file) throws SaxonApiException {
        return buildTransformer(buildDocument(new javax.xml.transform.stream.StreamSource(file)));
    }

    /**
     * Convenience method to build a XSLT transformer from a resource.
     *
     * @param uri The location of the resource
     * @return An executable XSLT
     * @throws Exception
     */
    static public XsltExecutable buildTransformer(URL url) throws SaxonApiException {
        return buildTransformer(buildDocument(new javax.xml.transform.stream.StreamSource(url.toExternalForm())));
    }

    /**
     * Convenience method to build a XSLT transformer from a resource.
     *
     * @param uri The location of the resource
     * @return An executable XSLT
     * @throws Exception
     */
    static public XsltExecutable buildTransformer(InputStream stream) throws SaxonApiException {
        return buildTransformer(buildDocument(new javax.xml.transform.stream.StreamSource(stream)));
    }
        
    /**
     * Wrap a DOM Node in a Saxon XDM node.
     */
    static public XdmNode wrapNode(Node node) {
        return getDocumentBuilder().wrap(node);
    }
    
    /**
     * Unwrap a DOM Node from a Saxon XDM node.
     */
    static public Node unwrapNode(XdmNode node) {
        return (Node)((VirtualNode)node.getUnderlyingNode()).getUnderlyingNode();
    }
    /* XPath2 utilities */

    static public XPathSelector xpathCompile(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        try {
            XPathCompiler xpc = getProcessor().newXPathCompiler();
            if (vars!=null) {
                for (Iterator iter = vars.keySet().iterator();iter.hasNext();) {
                    String name  = (String)iter.next();
                    xpc.declareVariable(new QName(name));
                }
            }
            if (nss!=null) {
                for (Iterator iter = nss.keySet().iterator();iter.hasNext();) {
                    String prefix  = (String)iter.next();
                    xpc.declareNamespace(prefix, nss.get(prefix));
                }
            }
            XPathSelector xps = xpc.compile(xp).load();
            xps.setContextItem(ctxt);
            if (vars!=null) {
                for (Iterator iter = vars.keySet().iterator();iter.hasNext();) {
                    String name  = (String)iter.next();
                    xps.setVariable(new QName(name),vars.get(name));
                }
            }
            return xps;
        } catch (SaxonApiException e) {
            logger.error("xpathCompile: xpath["+xp+"] failed: "+e);
            throw e;
        }
    }

    static public XPathSelector xpathCompile(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpathCompile(ctxt,xp,null,null);
    }

    static public XdmValue xpath(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        return xpathCompile(ctxt,xp,vars,nss).evaluate();
    }

    static public XdmValue xpath(XdmItem ctxt,String xp,Map<String,XdmValue> vars) throws SaxonApiException {
        return xpath(ctxt,xp,vars,null);
    }

    static public XdmValue xpath(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpath(ctxt,xp,null);
    }

    static public Iterator<XdmItem> xpathIterator(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        return xpathCompile(ctxt,xp,vars,nss).iterator();
    }

    static public Iterator<XdmItem> xpathIterator(XdmItem ctxt,String xp,Map<String,XdmValue> vars) throws SaxonApiException {
        return xpathIterator(ctxt,xp,vars,null);
    }

    static public Iterator<XdmItem> xpathIterator(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpathIterator(ctxt,xp,null);
    }

    static public XdmItem xpathSingle(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        return xpathCompile(ctxt,xp,vars,nss).evaluateSingle();
    }

    static public XdmItem xpathSingle(XdmItem ctxt,String xp,Map<String,XdmValue> vars) throws SaxonApiException {
        return xpathSingle(ctxt,xp,vars,null);
    }

    static public XdmItem xpathSingle(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpathSingle(ctxt,xp,null);
    }

    static public String xpath2string(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        String res = "";
        for (Iterator iter=xpathIterator(ctxt,xp,vars,nss);iter.hasNext();) {
            res += ((XdmItem)iter.next()).getStringValue();
        }
        return res;
    }

    static public String xpath2string(XdmItem ctxt,String xp,Map<String,XdmValue> vars) throws SaxonApiException {
        return xpath2string(ctxt,xp,vars,null);
    }

    static public String xpath2string(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpath2string(ctxt,xp,null,null);
    }

    static public boolean xpath2boolean(XdmItem ctxt,String xp,Map<String,XdmValue> vars,Map<String,String> nss) throws SaxonApiException {
        return xpathCompile(ctxt,xp,vars,nss).effectiveBooleanValue();
    }

    static public boolean xpath2boolean(XdmItem ctxt,String xp,Map<String,XdmValue> vars) throws SaxonApiException {
        return xpath2boolean(ctxt,xp,vars,null);
    }
   
    static public boolean xpath2boolean(XdmItem ctxt,String xp) throws SaxonApiException {
        return xpath2boolean(ctxt,xp,null,null);
    }
   
    /* Attributes */
    
    static public boolean hasAttribute(XdmItem ctxt,String attr) throws SaxonApiException {
        return Saxon.xpath2boolean(ctxt,"exists(@"+attr+")");
    }
    
    /* Attribute Value Templates */
    static protected Pattern AVTPattern = Pattern.compile("\\{+.*?\\}+");

    static public String avt(String avt,XdmItem ctxt,Map<String,XdmValue> vars) throws SaxonApiException {
        return avt(avt,ctxt,vars,true);
    }

    static public String avt(String avt,XdmItem ctxt,Map<String,XdmValue> vars,boolean unescape) throws SaxonApiException {
        String res = "";
        Matcher AVTMatcher = AVTPattern.matcher(avt);
        int start = 0;
        while (AVTMatcher.find()) {
            if (start < AVTMatcher.start())
                res += avt.substring(start,AVTMatcher.start());
            String grp = AVTMatcher.group();
            if (grp.startsWith("{{") && grp.endsWith("}}")) {
                if (unescape)
                    res += grp.substring(1,grp.length()-1);
                else
                    res += grp;
            } else {
                try {
                    res += Saxon.xpath2string(ctxt,grp.substring(1,grp.length()-1),vars);
                } catch(SaxonApiException e) {
                    logger.error("avt["+avt+"] failed: "+e);
                    throw e;
                }
            }
            start = AVTMatcher.end();
        }
        if (start < avt.length())
            res += avt.substring(start,avt.length());
        if (start > 0)
            logger.debug("AVT result["+res+"]");
        return res;
    }
    
    // save an XML 
    
    static public void save(Source source,File result) throws SaxonApiException {
        try {
            XsltTransformer transformer = buildTransformer(Saxon.class.getResource("/identity.xsl")).load();
            transformer.setSource(source);
            transformer.setDestination(getProcessor().newSerializer(result));
            transformer.transform();
            transformer.close();
        } catch (Exception ex) {
            throw new SaxonApiException(ex);
        }
    }
    
}
