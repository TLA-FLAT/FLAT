/*
 * Copyright (C) 2014 The Language Archive - Max Planck Institute for Psycholinguistics
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
package nl.mpi.tla.lat2fox;

import java.io.File;
import java.util.Collection;
import java.util.List;
import javax.xml.transform.TransformerFactory;
import javax.xml.transform.stream.StreamResult;
import javax.xml.transform.stream.StreamSource;
import joptsimple.OptionParser;
import joptsimple.OptionSet;
import net.sf.saxon.s9api.QName;
import net.sf.saxon.s9api.XdmAtomicValue;
import net.sf.saxon.s9api.XdmDestination;
import net.sf.saxon.s9api.XdmNode;
import net.sf.saxon.s9api.XsltExecutable;
import net.sf.saxon.s9api.XsltTransformer;
import nl.mpi.tla.schemanon.Message;
import nl.mpi.tla.schemanon.SchemAnon;
import nl.mpi.tla.schemanon.SaxonUtils;
import org.apache.commons.io.FileUtils;

/**
 * @author Menzo Windhouwer
 */
public class Main {
    
    private static void showHelp() {
        System.err.println("INF: lat2fox <options> -- <DIR>?");
        System.err.println("INF: <DIR>      source directory to recurse for CMD files (default: .)");
        System.err.println("INF: lat2fox options:");
        System.err.println("INF: -e=<EXT>   the extension of CMD records (default: cmdi)");
        System.err.println("INF: -r=<FILE>  load/store the relations map from/in this <FILE> (optional)");
        System.err.println("INF: -f=<DIR>   directory to store the FOX files (default: ./fox)");
        System.err.println("INF: -x=<DIR>   directory to store the FOX files with problems (default: ./fox-error)");
        System.err.println("INF: -i=<DIR>   replace source <DIR> by this <DIR> in the FOX files (optional)");
        System.err.println("INF: -n=<NUM>   create subdirectories to contain <NUM> FOX files (default: 0, i.e., no subdirectories)");
        System.err.println("INF: -c=<FILE>  file containing the mapping to collections (optional)");
        System.err.println("INF: -d=<FILE>  stylesheet containing the mapping from CMD to Dublin Core (recommended)");
        System.err.println("INF: -m=<FILE>  stylesheet containing the mapping from CMD to other (non CMD and non DC) metadata formats (optional)");
        System.err.println("INF: -o=<XPATH> XPath 2.0 expressions determining if the CMD should be offered via OAI-PMH");
        System.err.println("INF: -s=<NAME>  name of the server/repository used by OAI");
        System.err.println("INF: -v         validate the FOX files (optional)");
        System.err.println("INF: -l         lax check if a local resource exists (optional)");
    }

    public static void main(String[] args) {
        File   rfile = null;
        String dir = ".";
        String fdir = null;
        String idir = null;
        String xdir = null;
        String cext = "cmdi";
        String cfile = null;
        String dfile = null;
        String mfile = null;
        String oxp = null;
        String server = null;
        XdmNode collsDoc = null;
        boolean validateFOX = false;
        boolean laxResourceCheck = false;
        int ndir = 0;
        // check command line
        OptionParser parser = new OptionParser( "lve:r:f:i:x:n:c:d:m:o:s:?*" );
        OptionSet options = parser.parse(args);
        if (options.has("l"))
            laxResourceCheck = true;
        if (options.has("v"))
            validateFOX = true;
        if (options.has("e"))
            cext = (String)options.valueOf("e");
        if (options.has("r"))
            rfile = new File((String)options.valueOf("r"));
        if (options.has("f"))
            fdir = (String)options.valueOf("f");
        if (options.has("i"))
            idir = (String)options.valueOf("i");
        if (options.has("x"))
            xdir = (String)options.valueOf("x");
        if (options.has("c")) {
            cfile = (String)options.valueOf("c");
            File c = new File(cfile);
            if (!c.isFile()) {
                System.err.println("FTL: -c expects a <FILE> argument!");
                showHelp();
                System.exit(1);
            }
            if (!c.canRead()) {
                System.err.println("FTL: -c <FILE> argument isn't readable!");
                showHelp();
                System.exit(1);
            }
            try {
                collsDoc = SaxonUtils.buildDocument(new StreamSource(cfile));
            } catch(Exception ex) {
                System.err.println("FTL: can't read collection <FILE>["+cfile+"]: "+ex);
                ex.printStackTrace(System.err);
            }
        }
        if (options.has("d")) {
            dfile = (String)options.valueOf("d");
            File d = new File(dfile);
            if (!d.isFile()) {
                System.err.println("FTL: -d expects a <FILE> argument!");
                showHelp();
                System.exit(1);
            }
            if (!d.canRead()) {
                System.err.println("FTL: -d <FILE> argument isn't readable!");
                showHelp();
                System.exit(1);
            }
        }
        if (options.has("m")) {
            mfile = (String)options.valueOf("m");
            File m = new File(mfile);
            if (!m.isFile()) {
                System.err.println("FTL: -m expects a <FILE> argument!");
                showHelp();
                System.exit(1);
            }
            if (!m.canRead()) {
                System.err.println("FTL: -m <FILE> argument isn't readable!");
                showHelp();
                System.exit(1);
            }
        }
        if (options.has("n")) {
            try {
                ndir = Integer.parseInt((String)options.valueOf("n"));
            } catch(NumberFormatException e) {
                System.err.println("FTL: -n expects a numeric argument!");
                showHelp();
                System.exit(1);
            }
        }
        if (options.has("o")) {
            oxp = (String)options.valueOf("o");
        }
        if (options.has("s")) {
            server = (String)options.valueOf("s");
        }
        if (options.has("?")) {
            showHelp();
            System.exit(0);
        }
        
        List arg = options.nonOptionArguments();
        if (arg.size()>1) {
            System.err.println("FTL: only one source <DIR> argument is allowed!");
            showHelp();
            System.exit(1);
        }
        if (arg.size() == 1)
            dir = (String)arg.get(0);

        try {
            SaxonExtensionFunctions.registerAll(SaxonUtils.getProcessor().getUnderlyingConfiguration());
        } catch (Exception e) {
            System.err.println("ERR: couldn't register the Saxon extension functions: "+e);
            e.printStackTrace();
        }
        try {
            if (fdir == null)
                fdir = dir + "/fox";
            if (xdir == null)
                xdir = dir + "/fox-error";
            XdmNode relsDoc = null;
            if (rfile != null && rfile.exists()) {
                relsDoc = SaxonUtils.buildDocument(new StreamSource(rfile));
                System.err.println("DBG: loaded["+rfile.getAbsolutePath()+"]");
            } else {
                // create lookup document for relations
                XsltTransformer rels = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2rels.xsl")).load();
                rels.setParameter(new QName("ext"), new XdmAtomicValue(cext));
                rels.setParameter(new QName("dir"), new XdmAtomicValue("file:"+dir));
                rels.setSource(new StreamSource(Main.class.getResource("/null.xml").toString()));
                XdmDestination dest = new XdmDestination();
                rels.setDestination(dest);
                rels.transform();
                relsDoc = dest.getXdmNode();
                if (rfile != null) {
                    TransformerFactory.newInstance().newTransformer().transform(relsDoc.asSource(),new StreamResult(rfile));
                    System.err.println("DBG: saved["+rfile.getAbsolutePath()+"]");
                }
            }
            // Check the relations
            XsltTransformer rcheck = SaxonUtils.buildTransformer(Main.class.getResource("/checkRels.xsl")).load();
            rcheck.setParameter(new QName("rels-doc"), relsDoc);
            rcheck.setSource(new StreamSource(Main.class.getResource("/null.xml").toString()));
            XdmDestination dest = new XdmDestination();
            rcheck.setDestination(dest);
            rcheck.transform();
            //System.exit(0);
            // CMDI 2 FOX
            // create the fox dirs
            FileUtils.forceMkdir(new File(fdir));
            FileUtils.forceMkdir(new File(xdir));
            Collection<File> inputs = FileUtils.listFiles(new File(dir),new String[] {cext},true);
            // if there is a CMD 2 DC or 2 other XSLT include it
            XsltExecutable cmd2fox = null;
            if (dfile != null || mfile != null) {
                XsltTransformer inclCMD2DC = SaxonUtils.buildTransformer(Main.class.getResource("/inclCMD2DCother.xsl")).load();
                inclCMD2DC.setSource(new StreamSource(Main.class.getResource("/cmd2fox.xsl").toString()));
                if (dfile != null)
                    inclCMD2DC.setParameter(new QName("cmd2dc"),new XdmAtomicValue("file://"+(new File(dfile)).getAbsolutePath()));
                if (mfile != null)
                    inclCMD2DC.setParameter(new QName("cmd2other"),new XdmAtomicValue("file://"+(new File(mfile)).getAbsolutePath()));
                XdmDestination destination = new XdmDestination();
                inclCMD2DC.setDestination(destination);
                inclCMD2DC.transform();
                cmd2fox = SaxonUtils.buildTransformer(destination.getXdmNode());                
            } else
                cmd2fox = SaxonUtils.buildTransformer(Main.class.getResource("/cmd2fox.xsl"));
            int err = 0;
            int i = 0;
            int s = inputs.size();
            for (File input:inputs) {
                i++;
                if (!input.isHidden() && !input.getAbsolutePath().matches(".*/(corpman|sessions)/.*")) {
                    try {
                        XsltTransformer fox = cmd2fox.load();
                        //fox.setParameter(new QName("rels-uri"), new XdmAtomicValue("file:"+map.getAbsolutePath()));
                        fox.setParameter(new QName("rels-doc"), relsDoc);
                        fox.setParameter(new QName("conversion-base"), new XdmAtomicValue(dir));
                        if (idir != null)
                            fox.setParameter(new QName("import-base"), new XdmAtomicValue(idir));
                        fox.setParameter(new QName("fox-base"), new XdmAtomicValue(fdir));
                        fox.setParameter(new QName("lax-resource-check"),new XdmAtomicValue(laxResourceCheck));
                        if (collsDoc != null)
                            fox.setParameter(new QName("collections-map"), collsDoc);
                        if (server != null)
                            fox.setParameter(new QName("repository"), new XdmAtomicValue(server));
                        if (oxp != null)
                            fox.setParameter(new QName("oai-include-eval"), new XdmAtomicValue(oxp));
                        fox.setSource(new StreamSource(input));
                        XdmDestination destination = new XdmDestination();
                        fox.setDestination(destination);
                        fox.transform();
                        String fid = SaxonUtils.evaluateXPath(destination.getXdmNode(),"/*/@PID").evaluateSingle().getStringValue();
                        File out = new File(fdir + "/"+fid.replaceAll("[^a-zA-Z0-9]", "_")+".xml");
                        if (out.exists()) {
                            System.err.println("ERR:"+i+"/"+s+": FOX["+out.getAbsolutePath()+"] already exists!");
                            out = new File(xdir + "/lat-error-"+(++err)+".xml");
                            System.err.println("WRN:"+i+"/"+s+": saved to FOX["+out.getAbsolutePath()+"] instead!");
                        }
                        TransformerFactory.newInstance().newTransformer().transform(destination.getXdmNode().asSource(),new StreamResult(out));
                        System.err.println("DBG:"+i+"/"+s+": created["+out.getAbsolutePath()+"]");
                    } catch (Exception e) {
                        System.err.println("ERR:"+i+"/"+s+": "+e);
                        System.err.println("WRN:"+i+"/"+s+": skipping file["+input.getAbsolutePath()+"]");
                    }
                }
            }
            if (ndir > 0) {
                int n = 0;
                int d = 0;
                inputs = FileUtils.listFiles(new File(fdir),new String[] {"xml"},true);
                i = 0;
                s = inputs.size();
                for (File input:inputs) {
                    i++;
                    if (n == ndir)
                        n = 0;
                    n++;
                    FileUtils.moveFileToDirectory(input,new File(fdir+"/"+(n==1?++d:d)),true);
                    if (n==1)
                        System.err.println("DBG:"+i+"/"+s+": moved to dir["+fdir+"/"+d+"]");
                }
            }
            if (validateFOX) {
                SchemAnon tron = new SchemAnon(Main.class.getResource("/foxml1-1.xsd"),"ingest");
                inputs = FileUtils.listFiles(new File(fdir),new String[] {"xml"},true);
                i = 0;
                s = inputs.size();
                for (File input:inputs) {
                    i++;
                    // validate FOX
                    if (!tron.validate(input)) {
                        System.err.println("ERR:"+i+"/"+s+": invalid file["+input.getAbsolutePath()+"]");
                        for (Message msg : tron.getMessages()) {
                            System.out.println("" + (msg.isError() ? "ERR: " : "WRN: ") + i+"/"+s+": " + (msg.getLocation() != null ? "at " + msg.getLocation() : ""));
                            System.out.println("" + (msg.isError() ? "ERR: " : "WRN: ") + i+"/"+s+": " + msg.getText());
                        }
                    } else
                        System.err.println("DBG:"+i+"/"+s+": valid file["+input.getAbsolutePath()+"]");
                }
            }
        } catch(Exception ex) {
            System.err.println("FTL: "+ex);
            ex.printStackTrace(System.err);
        }
    }
}