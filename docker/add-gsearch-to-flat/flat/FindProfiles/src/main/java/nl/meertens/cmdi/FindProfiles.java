/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package nl.meertens.cmdi;

import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.util.Collection;
import java.util.HashSet;
import java.util.List;
import java.util.Set;
import java.util.regex.Pattern;
import javax.xml.namespace.QName;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import org.apache.commons.io.FileUtils;

import org.codehaus.stax2.XMLInputFactory2;
import org.codehaus.stax2.XMLStreamReader2;
import org.codehaus.stax2.evt.XMLEvent2;
/**
 *
 * @author menzowi
 */
public class FindProfiles {
    
    static final String CMD_NS = "http://www.clarin.eu/cmd/";
    static final String CR_URI = "http(s)?://catalog.clarin.eu/ds/ComponentRegistry/";
    static final String XSI_NS = "http://www.w3.org/2001/XMLSchema-instance";
    
    static final int ERROR = -1;
    static final int START = 0;
    static final int OPEN_CMD = 1;
    static final int OPEN_HEADER = 2;
    static final int OPEN_MDPROFILE = 3;
    static final int STOP = 9;
    
    private static void showHelp() {
        System.err.println("INF: findProfiles <options> -- <DIR>?");
        System.err.println("INF: <DIR>     source directory to recurse for CMD files (default: .)");
        System.err.println("INF: findProfiles options:");
        System.err.println("INF: -e=<EXT>  the extension of CMDI files (default: cmdi)");
        System.err.println("INF: -d        show debug info");
        System.err.println("INF: -v        be verbose");
    }

    
    public static void main(String[] args) throws FileNotFoundException {

        Boolean debug = false;
        Boolean verbose = false;
        String dir = ".";
        String ext = "cmdi";
        // check command line
        OptionParser parser = new OptionParser("dve:?*");
        OptionSet options = parser.parse(args);
        if (options.has("d"))
            debug = true;
        if (options.has("v"))
            verbose = true;
        if (options.has("e"))
            ext = (String)options.valueOf("e");
        if (options.has("?")) {
            showHelp();
            System.exit(0);
        }
        
        List arg = options.nonOptionArguments();
        if (arg.size()>1) {
            System.err.println("!FTL: only one source <DIR> argument is allowed!");
            showHelp();
            System.exit(1);
        }
        if (arg.size() == 1)
            dir = (String)arg.get(0);
        
        Set<String> profiles = new HashSet<String>();
        
        Pattern cr_rest = Pattern.compile("^.*"+CR_URI+"rest/registry/profiles/",Pattern.DOTALL);
        Pattern cr_ext  = Pattern.compile("/xsd.*$",Pattern.DOTALL);

        XMLInputFactory2 xmlif = (XMLInputFactory2) XMLInputFactory2.newInstance();
        xmlif.configureForConvenience();

        Collection<File> inputs = FileUtils.listFiles(new File(dir),new String[] {ext},true);
        int e = 0;
        int i = 0;
        int s = inputs.size();
        for (File input:inputs) {
            i++;
            if (verbose)
                System.err.println("?INF: "+i+"/"+s+": "+input);
            int state = START;
            int sdepth = 0;
            int depth = 0;
            XMLStreamReader2 xmlr = null;
            FileInputStream in = null;
            String profile = null;
            try {
                in = new FileInputStream(input);
                xmlr = (XMLStreamReader2) xmlif.createXMLStreamReader(in);
                while (state != STOP && state != ERROR) {
                    int eventType = xmlr.getEventType();
                    QName qn = null;
                    switch (eventType) {
                        case XMLEvent2.START_ELEMENT:
                            depth++;
                            qn = xmlr.getName();
                            break;
                        case XMLEvent2.END_ELEMENT:
                            qn = xmlr.getName();
                            break;
                    }
                    switch (state) {
                        case START:
                            switch (eventType) {
                                case XMLEvent2.START_ELEMENT:
                                    if (qn.getNamespaceURI().equals(CMD_NS) && qn.getLocalPart().equals("CMD")) {
                                        state = OPEN_CMD;
                                        sdepth = depth;
                                        String prof = xmlr.getAttributeValue(XSI_NS,"schemaLocation");
                                        if (prof!=null) {
                                            //if (prof.contains(CR_URI)) {
                                            if (cr_rest.matcher(prof).find()) {
                                                prof = cr_rest.matcher(prof).replaceFirst("");
                                                prof = cr_ext.matcher(prof).replaceFirst("");
                                                profile = prof;
                                                if (verbose || debug)
                                                    System.out.println("?"+(debug?"DBG":"INF")+": "+input+": xsi:schemaLocation["+prof+"]");
                                            } else
                                                System.err.println("!WRN: "+input+": xsi:schemaLocation["+prof+"] doesn't contain a reference to a CMD profile in CR!");
                                        }
                                    } else {
                                        System.err.println("!ERR: "+input+": no cmd:CMD root found!");
                                        state = ERROR;
                                    }
                                    break;
                                case XMLEvent2.END_DOCUMENT:
                                    System.err.println("!ERR: "+input+": no XML content found!");
                                    state = ERROR;
                                    break;
                            }       
                            break;
                        case OPEN_CMD:
                            switch (eventType) {
                                case XMLEvent2.START_ELEMENT:
                                    if (qn.getNamespaceURI().equals(CMD_NS) && qn.getLocalPart().equals("Header")) {
                                        state = OPEN_HEADER;
                                        sdepth = depth;
                                    } else {
                                        System.err.println("!ERR: "+input+": no cmd:CMD/cmd:Header found!");
                                        state = ERROR;
                                    }
                                    break;
                                case XMLEvent2.END_ELEMENT:
                                    if (qn.getNamespaceURI().equals(CMD_NS) && qn.getLocalPart().equals("CMD") && sdepth == depth) {
                                        System.err.println("!ERR: "+input+": no cmd:CMD/cmd:Header found!");
                                        state = ERROR;
                                    }
                                    break;
                            }       
                            break;
                        case OPEN_HEADER:
                            switch (eventType) {
                                case XMLEvent2.START_ELEMENT:
                                    if (qn.getNamespaceURI().equals(CMD_NS) && qn.getLocalPart().equals("MdProfile") && sdepth+1==depth) {
                                        state = OPEN_MDPROFILE;
                                    }
                                    break;
                                case XMLEvent2.END_ELEMENT:
                                    if (qn.getNamespaceURI().equals(CMD_NS) && qn.getLocalPart().equals("Header") && sdepth == depth) {
                                        System.err.println("!"+(profile==null?"ERR":"WRN")+": "+input+": no cmd:CMD/cmd:Header/cmd:MdProfile found!");
                                        state = ERROR;
                                    }
                                    break;
                            }       
                            break;
                        case OPEN_MDPROFILE:
                            switch (eventType) {
                                case XMLEvent2.CHARACTERS:
                                    String prof = xmlr.getText();
                                    prof = cr_rest.matcher(prof).replaceFirst("");
                                    prof = cr_ext.matcher(prof).replaceFirst("");
                                    if (verbose || debug)
                                        System.out.println("?"+(debug?"DBG":"INF")+": "+input+": MdProfile["+prof+"]");
                                    if (profile == null)
                                        profile = prof;
                                    else if (!prof.equals(profile))
                                        System.out.println("!WRN: "+input+": MdProfile["+prof+"] and xsi:schemaLocation["+profile+"] contradict!");
                                    state = STOP;
                                    break;
                                default:
                                    state = STOP;
                                    break;
                            }
                            break;
                    }
                    switch (eventType) {
                        case XMLEvent2.END_ELEMENT:
                            depth--;
                            break;
                    }
                    eventType = xmlr.next();
                }
            } catch (Exception ex) {
                System.err.println("!ERR: "+input+": "+ex);
                ex.printStackTrace(System.err);
                state = ERROR;
            } finally {
                try {
                    xmlr.close();
                    in.close();
                } catch (Exception ex) {
                    System.err.println("!ERR: "+input+": "+ex);
                    ex.printStackTrace(System.err);
                    state = ERROR;
                }
            }
            if (profile != null)
                profiles.add(profile);
            if (state == ERROR)
                e++;
        }
        for (String profile:profiles) {
            System.out.println(profile);
        }
        System.exit(e);
    }
}
