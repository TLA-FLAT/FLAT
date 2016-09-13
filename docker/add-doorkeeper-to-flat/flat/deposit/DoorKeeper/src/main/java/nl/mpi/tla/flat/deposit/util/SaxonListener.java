/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.mpi.tla.flat.deposit.util;

import javax.xml.transform.ErrorListener;
import javax.xml.transform.SourceLocator;
import javax.xml.transform.TransformerException;
import net.sf.saxon.s9api.MessageListener;
import net.sf.saxon.s9api.XdmNode;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.slf4j.MDC;

/**
 *
 * @author menzowi
 */
public class SaxonListener implements MessageListener, ErrorListener {
    
    private static final Logger logger = LoggerFactory.getLogger(Saxon.class.getName());
    
    protected String type = "Saxon";
    protected String sip  = null;

    public SaxonListener() {
        this(null,null);
    }

    public SaxonListener(String type) {
        this(type,null);
    }

    public SaxonListener(String type,String sip) {
        System.err.println("!MENZO: setup SaxonErrorListener["+type+"]["+sip+"]");
        if (type != null)
            this.type = type;
        if (sip != null)
            this.sip = sip;
    }
    
    protected void setSIP() {
        if (this.sip != null)
            if (MDC.get("sip")==null)
                MDC.put("sip",sip);
        System.err.println("!MENZO: SaxonErrorListener.sip["+MDC.get("sip")+"]");
    }
    
    protected boolean handleMessage(String msg, String loc, Exception e) {
        if (msg.startsWith("INF: "))
            logger.info(type+": "+msg.replace("INF: ", ""));
        else if (msg.startsWith("WRN: "))
            logger.warn(type+"["+loc+"]: "+msg.replace("WRN: ", ""), e);
        else if (msg.startsWith("ERR: "))
            logger.error(type+"["+loc+"]: "+msg.replace("ERR: ", ""), e);
        else if (msg.startsWith("DBG: "))
            logger.debug(type+"["+loc+"]: "+msg.replace("DBG: ", ""), e);
        else
            return false;
        return true;
    }
    
    protected boolean handleException(TransformerException te) {
        return handleMessage(te.getMessage(), te.getLocationAsString(), te);
    }

    @Override
    public void warning(TransformerException te) throws TransformerException {
        setSIP();
        if (!handleException(te))
            logger.warn(type+": "+te.getMessageAndLocation(), te);
    }

    @Override
    public void error(TransformerException te) throws TransformerException {
        setSIP();
        if (!handleException(te))
            logger.error(type+": "+te.getMessageAndLocation(), te);
    }

    @Override
    public void fatalError(TransformerException te) throws TransformerException {
        setSIP();
        if (!handleException(te))
            logger.error(type+": "+te.getMessageAndLocation(), te);
    }
    
    protected String getLocation(SourceLocator sl) {
        if (sl.getColumnNumber()<0)
            return "-1";
        return sl.getSystemId()+":"+sl.getLineNumber()+":"+sl.getColumnNumber();
    }

    @Override
    public void message(XdmNode xn, boolean bln, SourceLocator sl) {
        setSIP();
        if (!handleMessage(xn.getStringValue(),getLocation(sl),null)) {
            if (bln)
                logger.error(type+"["+getLocation(sl)+"]: "+xn.getStringValue());
            else
                logger.info(type+"["+getLocation(sl)+"]: "+xn.getStringValue());
        }
    }
}